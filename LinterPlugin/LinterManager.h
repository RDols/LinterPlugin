#pragma once  

#include "..\notepad-plus-plus\PowerEditor\src\MISC\PluginsManager\Notepad_plus_msgs.h"
#include "DecoderBase.h"
#include "LinterData.h"

class CLinterPlugin;

//enum NLintType
//{
//  LT_CHECK = 0,
//  LT_CORRECT,
//  LT_COUNT
//};

struct SCommandInfo
{
  LangType LanguageType;
  //NLintType LineType;
  std::wstring Command;
  CDecoderBase* OutputDecoder;
  CDecoderBase* InlineDecoder;
};

class CLinterManager
{
public: //C onstructor
  CLinterManager();
  virtual ~CLinterManager();
  
public: // Interface
  void Init(CLinterPlugin* Parent, const TCHAR* ModulePath);
  void SetFileLanguage(LangType FileLanguage) { mCurrentFileLanguage = FileLanguage; };
  void StopErrorChecking();
  bool GetNewErrors(std::vector<SLintError>& Errors);

  void LintContent(const std::string& doctxt, int Delay, bool ForceChanged);
  void FormatDocument(std::string& doctxt);

public: //Thread Functions
  static VOID CALLBACK StartCheckThread(PVOID lpParam, BOOLEAN TimerOrWaitFired);
  static void OnCheckThreadStart(PVOID lpParam);

protected: // Help Functions
  void _LintContent();

public: //Help Functions
  static bool ExecuteCommand(const SCommandInfo& CommandInfo, const std::string& DocTxt, std::string& OutputStringA, std::string ErrorStringA, std::vector<SLintError>& RuntimeErrorList);
  static void WriteTempFile(TCHAR* TempFilePath, const std::string& DocTxt);
  static void DeleteTempFile(TCHAR* TempFilePath);
  static stringT ExpandCommand(stringT Command, TCHAR* TempFilePath);
  static int Replace(stringT& str, const stringT& oldStr, const stringT& newStr);

protected: // Variables
  CRITICAL_SECTION mCriticalSection_Busy;
  std::vector<SCommandInfo> mLinterCommands;
  std::vector<SCommandInfo> mFormatterCommands;
  CLinterPlugin* mParent;
  static TCHAR mModulePath[MAX_PATH];

  HANDLE mDelayCheckTimer;
  LangType mCurrentFileLanguage;
  std::string mDocTxt;

  bool mNewDataAvailable;
  bool mNewDataAvailableForced;
  std::vector<SLintError> mErrors;
};