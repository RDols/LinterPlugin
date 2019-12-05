//

#include "stdafx.h"
#include "LinterManager.h"
#include "LinterPlugin.h"
#include "LuaCheckPlainDecoder.h"

#include <thread>
#include <atlcomcli.h>

#include <algorithm>

TCHAR CLinterManager::mModulePath[MAX_PATH];

CLinterManager::CLinterManager()
  : mNewDataAvailable(false)
  , mNewDataAvailableForced(false)
  , mDelayCheckTimer(0)
  , mCurrentFileLanguage(L_EXTERNAL)
{
  SCommandInfo NewCommand;
  NewCommand.LanguageType = L_LUA;
  //NewCommand.LineType = LT_CHECK;
  //NewCommand.Command = L"luacheck.exe --codes --ranges --formatter plain -d:\DoesntExist.tmp";
  NewCommand.Command = L"%PLUGINDIR%luacheck.exe --codes --ranges --formatter plain - %STD%";
  //NewCommand.Command = L"luacheck.exe --codes --ranges --formatter plain %FILE%";
  //NewCommand.Command = L"D:\\lua\\luacheck.exe --formatter d:\\lua\\dolsexport";
  //NewCommand.Command = L"D:\\lua\\luacheck.exe --codes --ranges --formatter plain";
  NewCommand.OutputDecoder = new CLuaCheckPlainDecoder();
  NewCommand.InlineDecoder = new CLuaCheckPlainDecoder(true);
  mLinterCommands.push_back(NewCommand);

  NewCommand.OutputDecoder = NULL;
  NewCommand.InlineDecoder = NULL;
  NewCommand.Command = L"%PLUGINDIR%lua-format.exe -si -c lua-format.cfg %STD%";
  mFormatterCommands.push_back(NewCommand);

  InitializeCriticalSection(&mCriticalSection_Busy);
  CoInitialize(0);
}

CLinterManager::~CLinterManager()
{
  CoUninitialize();
  DeleteCriticalSection(&mCriticalSection_Busy);
}

void CLinterManager::Init(CLinterPlugin* Parent, const TCHAR* ModulePath)
{
  mParent = Parent;
  lstrcpy(mModulePath, ModulePath);
}

void CLinterManager::StopErrorChecking()
{
  EnterCriticalSection(&mCriticalSection_Busy);

  if (mDelayCheckTimer)
    DeleteTimerQueueTimer(NULL, mDelayCheckTimer, NULL);
  mDelayCheckTimer = 0;

  mErrors.clear();
  mNewDataAvailable = true;
  LeaveCriticalSection(&mCriticalSection_Busy);
}

bool CLinterManager::GetNewErrors(std::vector<SLintError>& Errors)
{
  if (!mNewDataAvailable) return false;
  if (!TryEnterCriticalSection(&mCriticalSection_Busy)) return false;
  Errors = mErrors;
  mNewDataAvailable = false;
  LeaveCriticalSection(&mCriticalSection_Busy);
  return true;
}

void CLinterManager::FormatDocument(std::string& doctxt)
{
  for (std::vector<SCommandInfo>::iterator it = mFormatterCommands.begin(); it != mFormatterCommands.end(); it++)
  {
    if (it->LanguageType == mCurrentFileLanguage)
    {
      std::string OutputStringA;
      std::string ErrorStringA;
      ExecuteCommand(*it, mDocTxt, OutputStringA, ErrorStringA, mErrors);
      doctxt = OutputStringA;
    }
  }
}

void CLinterManager::LintContent(const std::string& doctxt, int Delay, bool ForceChanged)
{
  if (doctxt.empty()) return;
  EnterCriticalSection(&mCriticalSection_Busy);

  mNewDataAvailableForced = ForceChanged;
  
  mDocTxt = doctxt;
  LeaveCriticalSection(&mCriticalSection_Busy);

  if (mDelayCheckTimer)
    DeleteTimerQueueTimer(NULL, mDelayCheckTimer, NULL);
  CreateTimerQueueTimer(&mDelayCheckTimer, NULL, (WAITORTIMERCALLBACK)CLinterManager::StartCheckThread, this, Delay*1000, 0, 0);
}

VOID CALLBACK CLinterManager::StartCheckThread(PVOID lpParam, BOOLEAN /*TimerOrWaitFired*/)
{
  std::thread WorkerThread(CLinterManager::OnCheckThreadStart, lpParam);
  WorkerThread.detach();
}

void CLinterManager::OnCheckThreadStart(PVOID lpParam)
{
  CLinterManager* luaCheck = (CLinterManager*)lpParam;
  luaCheck->_LintContent();
}

void CLinterManager::_LintContent()
{
  EnterCriticalSection(&mCriticalSection_Busy);
  DeleteTimerQueueTimer(NULL, mDelayCheckTimer, NULL);
  mDelayCheckTimer = 0;
  mErrors.clear();

  mNewDataAvailable = mNewDataAvailableForced;
  mNewDataAvailableForced = false;
  for (std::vector<SCommandInfo>::iterator it = mLinterCommands.begin(); it != mLinterCommands.end(); it++)
  {
    if (it->LanguageType == mCurrentFileLanguage)
    {
      std::string OutputStringA;
      std::string ErrorStringA;
      ExecuteCommand(*it, mDocTxt, OutputStringA, ErrorStringA, mErrors);
      if (it->OutputDecoder->DecodeErrors(OutputStringA))
        mNewDataAvailable = true;

      it->OutputDecoder->AppendErrors(mErrors);

      if (it->InlineDecoder)
      {
        OutputStringA.clear();
        ErrorStringA.clear();
        std::string disabledOutputTxt;
        it->InlineDecoder->DisableInlines(mDocTxt, disabledOutputTxt);
        ExecuteCommand(*it, disabledOutputTxt, OutputStringA, ErrorStringA, mErrors);
        if (it->InlineDecoder->DecodeErrors(OutputStringA))
          mNewDataAvailable = true;

        it->InlineDecoder->AppendErrors(mErrors);
      }
    }
    else
    {
      it->OutputDecoder->Clear();
    }
  }

  std::sort(mErrors.begin(), mErrors.end());
  

  LeaveCriticalSection(&mCriticalSection_Busy);
  //mParent->ShowErrors(false);
}

bool CLinterManager::ExecuteCommand(const SCommandInfo& CommandInfo, const std::string& DocTxt, std::string& OutputStringA, std::string ErrorStringA, std::vector<SLintError>& RuntimeErrorList)
{
  stringT CmdCommand(CommandInfo.Command);
  TCHAR TempFilePath[MAX_PATH];
  OutputStringA.clear();
  ErrorStringA.clear();

  bool useStdInput(true);
  if (Replace(CmdCommand, _T("%STD%"), _T("")) == 0)
  {
    WriteTempFile(TempFilePath, DocTxt);
    useStdInput = false;
  }

  CmdCommand = ExpandCommand(CmdCommand, TempFilePath);

  SECURITY_ATTRIBUTES security;
  security.nLength = sizeof(SECURITY_ATTRIBUTES);
  security.bInheritHandle = TRUE;
  security.lpSecurityDescriptor = NULL;

  HANDLE outputRead(0);
  HANDLE outputWrite(0);
  CreatePipe(&outputRead, &outputWrite, &security, 0);
  SetHandleInformation(outputRead, HANDLE_FLAG_INHERIT, 0);

  HANDLE errorRead(0);
  HANDLE errorWrite(0);
  CreatePipe(&errorRead, &errorWrite, &security, 0);
  SetHandleInformation(errorRead, HANDLE_FLAG_INHERIT, 0);

  HANDLE inputRead(0);
  HANDLE inputWrite(0);
  CreatePipe(&inputRead, &inputWrite, &security, 0);
  SetHandleInformation(inputWrite, HANDLE_FLAG_INHERIT, 0);

  STARTUPINFO startInfo = { 0 };
  startInfo.cb = sizeof(STARTUPINFO);
  startInfo.hStdError = errorWrite;
  startInfo.hStdOutput = outputWrite;
  startInfo.hStdInput = inputRead;
  startInfo.dwFlags |= STARTF_USESTDHANDLES;

  PROCESS_INFORMATION processInfo = { 0 };
    BOOL isSuccess = CreateProcess(NULL,
    const_cast<TCHAR *>(CmdCommand.c_str()),   // command line
    NULL,                                        // process security attributes
    NULL,                                        // primary thread security attributes
    TRUE,                                        // handles are inherited
    CREATE_NO_WINDOW,                            // creation flags
    NULL,                                        // use parent's environment
    mModulePath,                                 // use DLL directory
    &startInfo,                                  // STARTUPINFO pointer
    &processInfo);                               // receives PROCESS_INFORMATION

  if (!isSuccess)
  {
    //ERROR_FILE_NOT_FOUND  ERROR_ELEVATION_REQUIRED
    //Cannot execute command. CmdCommand is incorrect.
    DWORD err = GetLastError();
    char errMessage[256];
    /*DWORD errFormat =*/ FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), errMessage, 255, NULL);
    SLintError newErrorEntry;
    newErrorEntry.m_severity = SV_DEBUG;
    newErrorEntry.m_error_code = err;
    newErrorEntry.m_subject = "CreateProcess";
    newErrorEntry.m_message = errMessage;
    RuntimeErrorList.push_back(newErrorEntry);
    DeleteTempFile(TempFilePath);
    return false;
  }

  //write to pipe
  if (useStdInput)
  {
    DWORD bytes(0);
    WriteFile(inputWrite, DocTxt.c_str(), static_cast<DWORD>(DocTxt.size() * sizeof(DocTxt[0])), &bytes, 0);
  }

  //close input pipe
  CloseHandle(inputRead);
  CloseHandle(inputWrite);

  //close process
  CloseHandle(processInfo.hThread);

  //close handles to process
  CloseHandle(errorWrite);
  CloseHandle(outputWrite);
 
  DWORD readBytes;
  std::string buffer;
  buffer.resize(4096);
  while(true)
  {
    isSuccess = ReadFile(outputRead, &buffer[0], (DWORD) buffer.size(), &readBytes, NULL);
    if (!isSuccess || readBytes == 0)
    {
      DWORD error = GetLastError();
      if (error == ERROR_BROKEN_PIPE) // Finished reading pipe, normal end
        break;

      DWORD err = GetLastError();
      char errMessage[256];
      /*DWORD errFormat =*/ FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), errMessage, 255, NULL);
      SLintError newErrorEntry;
      newErrorEntry.m_severity = SV_DEBUG;
      newErrorEntry.m_error_code = err;
      newErrorEntry.m_subject = "ReadFile output";
      newErrorEntry.m_message = errMessage;
      RuntimeErrorList.push_back(newErrorEntry);
      break;
    }
    OutputStringA += std::string(&buffer[0], readBytes);
  }
  while (true)
  {
    isSuccess = ReadFile(errorRead, &buffer[0], (DWORD)buffer.size(), &readBytes, NULL);
    if (!isSuccess || readBytes == 0)
    {
      DWORD error = GetLastError();
      if (error == ERROR_BROKEN_PIPE) // Finished reading pipe, normal end
        break;

      DWORD err = GetLastError();
      char errMessage[256];
      /*DWORD errFormat =*/ FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), errMessage, 255, NULL);
      SLintError newErrorEntry;
      newErrorEntry.m_severity = SV_DEBUG;
      newErrorEntry.m_error_code = err;
      newErrorEntry.m_subject = "ReadFile error";
      newErrorEntry.m_message = errMessage;
      RuntimeErrorList.push_back(newErrorEntry);
      break;
    }
    ErrorStringA += std::string(&buffer[0], readBytes);
  }

  //close output
  CloseHandle(errorRead);
  CloseHandle(outputRead);

  DWORD exitCode(0);
  if (GetExitCodeProcess(processInfo.hProcess, &exitCode))
  {
    std::stringstream ErrorTxt;
    ErrorTxt << "Process ended with exitcode: " << exitCode << "\r\n";
    ErrorStringA.append(ErrorTxt.str());

    SLintError newErrorEntry;
    newErrorEntry.m_severity = SV_DEBUG;
    newErrorEntry.m_error_code = exitCode;
    newErrorEntry.m_subject = "GetExitCodeProcess";
    newErrorEntry.m_message = "Process normally ended with exitcode.";
    RuntimeErrorList.push_back(newErrorEntry);
  }
  else
  {
    DWORD err = GetLastError();
    char errMessage[256];
    /*DWORD errFormat =*/ FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), errMessage, 255, NULL);
    SLintError newErrorEntry;
    newErrorEntry.m_severity = SV_DEBUG;
    newErrorEntry.m_error_code = err;
    newErrorEntry.m_subject = "GetExitCodeProcess";
    newErrorEntry.m_message = errMessage;
    RuntimeErrorList.push_back(newErrorEntry);
  }

  CloseHandle(processInfo.hProcess);
  DeleteTempFile(TempFilePath);
  return true;
}

void CLinterManager::WriteTempFile(TCHAR* TempFilePath, const std::string& DocTxt)
{
  _ttmpnam_s(TempFilePath, MAX_PATH);
  
  HANDLE fileHandle = CreateFile(TempFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_TEMPORARY, NULL);
  if (fileHandle == INVALID_HANDLE_VALUE) return;

  DWORD bytes(0);
  WriteFile(fileHandle, DocTxt.c_str(), static_cast<DWORD>(DocTxt.size() * sizeof(DocTxt[0])), &bytes, 0);
  CloseHandle(fileHandle);
}

void CLinterManager::DeleteTempFile(TCHAR* TempFilePath)
{
  if (_tcslen(TempFilePath) > 0)
    _wunlink(TempFilePath);

  TempFilePath[0] = 0;
}

stringT CLinterManager::ExpandCommand(stringT Command, TCHAR* TempFilePath)
{
  stringT ExpandedCommand = Command;

  int count(0);
  count = Replace(ExpandedCommand, _T("%FILE%"), TempFilePath);
  count = Replace(ExpandedCommand, _T("%PLUGINDIR%"), mModulePath);

  return ExpandedCommand;
}

int CLinterManager::Replace(stringT& str, const stringT& oldStr, const stringT& newStr)
{
  int count(0);
  stringT::size_type pos = 0u;
  while ((pos = str.find(oldStr, pos)) != stringT::npos)
  {
    str.replace(pos, oldStr.length(), newStr);
    pos += newStr.length();
    ++count;
  }
  return count;
}