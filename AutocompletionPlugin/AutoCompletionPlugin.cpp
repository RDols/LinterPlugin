// AutoCompletionPlugin.cpp : Defines the exported functions for the DLL application.

#include "stdafx.h"

#include "AutoCompletionPlugin.h"
#include "resource.h"

CAutoCompletionPlugin* Plugin = new CAutoCompletionPlugin;
INppDll* nppDll = Plugin;

#define PLUGIN_MENU_ENABLE              1

#define MSG_DELAYED_AUTOCOMPLETION      WM_USER + 7330

const std::string WhiteSpace = "\r\n\t\"\' ()[]{}";

CAutoCompletionPlugin::CAutoCompletionPlugin()
{
  mPluginName = L"AutoCompletion";
  mPluginShortName = "AutoCompletion";
  AddMenuItem(PLUGIN_MENU_ENABLE, _T("Enable"), CAutoCompletionPlugin::EnablePluginStatic, NULL, true, 0);
}

CAutoCompletionPlugin ::~CAutoCompletionPlugin()
{
}

void CAutoCompletionPlugin::PluginInit(HMODULE Module)
{
  __super::PluginInit(Module);
}

void CAutoCompletionPlugin::beNotified(SCNotification* Notification)
{
  __super::beNotified(Notification);

  switch (Notification->nmhdr.code)
  {
  case NPPN_READY:
    Init();
    break;
  case NPPN_FILEOPENED:
  case NPPN_LANGCHANGED:
  case NPPN_DOCORDERCHANGED:
  case NPPN_BUFFERACTIVATED:
    mDummy = true;
    break;
  case SCN_MODIFIED:
    if (Notification->modificationType & (SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT))
    {

    }
    break;
  case SCN_AUTOCCOMPLETED:
    break;
  case SCN_CHARADDED:
    OnAddChar(Notification->ch);
    break;
  case SCN_USERLISTSELECTION:
    if (Notification->listType == 10)
    {
      mListOpen = false;
      ReplaceCurrentWord(Notification->text);
    }
    break;
  case SCN_AUTOCCANCELLED:
    mListOpen = false;
    break;

  case SCN_UPDATEUI:
    break;

  case MSG_DELAYED_AUTOCOMPLETION:
    OnAutoCompleteStart(false);
    break;

  default:
    break;
  }
  _RPT1(0, "CAutoCompletionPlugin::beNotified code:%d\r\n", Notification->nmhdr.code);
}

void CAutoCompletionPlugin::EnablePlugin()
{
	mPluginEnabled = !mPluginEnabled;
	SendApp(NPPM_SETMENUITEMCHECK, GetNppMenuId(PLUGIN_MENU_ENABLE), mPluginEnabled ? TRUE : FALSE);
}

void CAutoCompletionPlugin::Init()
{
  LoadLists();
  mInitialized = true;
}

void CAutoCompletionPlugin::LoadLists()
{
  std::string filename = mModulePathA;

  mKeyWordList.Append(filename + "Bindings.csv");
  mKeyWordList.Append(filename + "lua.csv");
  mKeyWordList.Append(filename + "lua2.csv");
}


void CAutoCompletionPlugin::OnAutoCompleteStart(bool Force)
{
  if (!GetCurrentWord())
    return;

  if (!Force && !mListOpen && mCurrentWord.size() < 4)
    return;

  CreateDocumentList();
  std::string list = mKeyWordList.GetCompletionList(mCurrentWord);
  if (list.size() <= 0)
    return;

  SendEditor(SCI_AUTOCSETMAXHEIGHT, 50);
  SendEditor(SCI_AUTOCSETSEPARATOR, WPARAM('|'));
  SendEditor(SCI_AUTOCSETIGNORECASE, true);
  SendEditor(SCI_USERLISTSHOW, 10, reinterpret_cast<LPARAM>(list.c_str()));
  mListOpen = true;
}


void CAutoCompletionPlugin::DelayedAutocompletion(uint32_t delayms)
{
  if (mDelayCheckTimer)
    DeleteTimerQueueTimer(NULL, mDelayCheckTimer, NULL);

  CreateTimerQueueTimer(&mDelayCheckTimer, NULL, (WAITORTIMERCALLBACK)CAutoCompletionPlugin::SendAutocompletion, this, delayms, 0, 0);
}

void CAutoCompletionPlugin::SendAutocompletion(PVOID lpParam, BOOLEAN /*TimerOrWaitFired*/)
{
  CAutoCompletionPlugin* plugin = (CAutoCompletionPlugin*)lpParam;
  plugin->StartAutoCompletionAsync();
}

void CAutoCompletionPlugin::StartAutoCompletionAsync()
{
  NMHDR nmh;
  nmh.code = MSG_DELAYED_AUTOCOMPLETION;
  nmh.idFrom = 0;
  nmh.hwndFrom = 0;
  SendApp(WM_NOTIFY, nmh.idFrom, (LPARAM)&nmh);
}

void CAutoCompletionPlugin::OnAddChar(int /*ch*/)
{
  if (!mInitialized) return;

  DelayedAutocompletion(100);
  

  std::string list;
  SendEditor(SCI_AUTOCSETMAXHEIGHT, -1);
  //SendEditor(SCI_USERLISTSHOW, 10, reinterpret_cast<LPARAM>(list.c_str()));
  //SendEditor(SCI_AUTOCCANCEL);
}


bool CAutoCompletionPlugin::GetCurrentWord()
{
  int64_t curPos = (int64_t) SendEditor(SCI_GETCURRENTPOS);
  int64_t docLength = (int64_t) SendEditor(SCI_GETLENGTH);

  const int32_t bufSizeSize = 1024;
  int64_t TextStart = max(0, curPos - (bufSizeSize / 2) + 5);
  int64_t TextEnd = min(docLength, curPos + (bufSizeSize / 2) - 5);
  std::string buffer = GetTextRange(TextStart, TextEnd);
  
  int64_t WordStart = curPos - TextStart;
  while (WordStart > 1)
  {
    if (WhiteSpace.find(buffer[WordStart - 1]) != std::string::npos)
      break;
    --WordStart;
  }

  int64_t WordEnd = curPos - TextStart;
  while (WordEnd < TextEnd)
  {
    if (WhiteSpace.find(buffer[WordEnd]) != std::string::npos)
      break;
    ++WordEnd;
  }

  if (WordEnd - WordStart <= 0)
    return false;

  mCurrentWordStart = WordStart + TextStart;
  mCurrentWordEnd = WordEnd + TextStart;
  mCurrentWord = buffer.substr(WordStart, WordEnd - WordStart);  
  return true;
}


void CAutoCompletionPlugin::CreateDocumentList()
{
  mKeyWordList.ClearDocumentList();

  int64_t docLength = (int64_t)SendEditor(SCI_GETLENGTH);

  int flags = SCFIND_WORDSTART | SCFIND_REGEXP; // | SCFIND_MATCHCASE | SCFIND_REGEXP | SCFIND_POSIX;
  SendEditor(SCI_SETSEARCHFLAGS, flags);
  SendEditor(SCI_SETTARGETRANGE, 0, docLength);

  // Todo: escape . in mCurrentWord
  std::string expr("[^ \\t\\n\\r,;:\"(){}=<>'+!\\[\\]]*");
  expr += mCurrentWord;
  expr += "[^ \\t\\n\\r,;:\"(){}=<>'+!\\[\\]]*";

  int64_t posFind = SendEditor(SCI_SEARCHINTARGET, (WPARAM) expr.size(), (LPARAM) expr.c_str());
  while (posFind >= 0)
  {
    int64_t wordStart = (int64_t) SendEditor(SCI_GETTARGETSTART);
    int64_t wordEnd = (int64_t) SendEditor(SCI_GETTARGETEND);
    if (wordStart != mCurrentWordStart)
    {      
      std::string word = GetTextRange(wordStart, wordEnd);
      mKeyWordList.AddToDocumentList(word);
    }

    SendEditor(SCI_SETTARGETRANGE, wordEnd, docLength);
    posFind = SendEditor(SCI_SEARCHINTARGET, (WPARAM)expr.size(), (LPARAM)expr.c_str());
  }
}

bool CAutoCompletionPlugin::ReplaceCurrentWord(const char* text)
{
  SendEditor(SCI_SETSEL, mCurrentWordStart, mCurrentWordEnd);
  SendEditor(SCI_REPLACESEL, 0, reinterpret_cast<LPARAM>(text));
  //StartAutoCompletionAsync();
  DelayedAutocompletion(10);  //can't start immediatly, current one is still be canceled.

  return true;
}