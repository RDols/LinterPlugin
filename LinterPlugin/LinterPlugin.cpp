// LinterPlugin.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "LinterPlugin.h"
#include "Resource/MarkerLuaExclamation.xpm"
#include "resource.h"

//#define MARKER_STYLE_ID_WARNING     INDIC_CONTAINER + 2
//#define MARKER_STYLE_ID_HIGHLIGHT   INDIC_CONTAINER + 3
#define INDICATOR_STYLE_ID_WARNING     INDIC_CONTAINER + 2
//#define INDICATOR_STYLE_ID_HIGHLIGHT   INDIC_CONTAINER + 3

CLinterPlugin* Plugin = new CLinterPlugin;
INppDll* nppDll = Plugin;

#define PLUGIN_MENU_SEPERATOR_FUNCTION  NULL
#define PLUGIN_MENU_SEPERATOR           1
#define PLUGIN_MENU_ENABLE              2
#define PLUGIN_MENU_CHECK_DOCUMENT      3
#define PLUGIN_MENU_SHOW_RESULT_LIST    4
#define PLUGIN_MENU_SHOW_FUNCTION_LIST  5
#define PLUGIN_MENU_SHOW_CONFIG         6

CLinterPlugin::CLinterPlugin()
  : mResultListDlg(this)
  , mFunctionListCtrl(this)
  , mConfigDlg(this)
{
  mPluginName = L"Linter Plugin";
  mPluginShortName = "LinterPlugin";
  //AddMenuItem(PLUGIN_MENU_SEPERATOR, _T(""), PLUGIN_MENU_SEPERATOR_FUNCTION, NULL, false, 0);
  AddMenuItem(PLUGIN_MENU_ENABLE, _T("Enable"), CLinterPlugin::EnablePluginStatic, NULL, false, 0);
  AddMenuItem(PLUGIN_MENU_CHECK_DOCUMENT, _T("Check Document"), CLinterPlugin::OnMenuCheckDocumentStatic, NULL, false, 0);
  AddMenuItem(PLUGIN_MENU_SHOW_RESULT_LIST, _T("Show Error List"), CLinterPlugin::OnMenuShowResultListDlgStatic, NULL, false, IDB_BITMAP_LUALINT);
  AddMenuItem(PLUGIN_MENU_SHOW_FUNCTION_LIST, _T("Show Function List"), CLinterPlugin::OnMenuShowFunctionListDlgStatic, NULL, false, IDB_BITMAP_LUA_LIST);
  AddMenuItem(PLUGIN_MENU_SHOW_CONFIG, _T("Configuration..."), CLinterPlugin::OnMenuShowConfigutationDlgStatic, NULL, false, 0);

  
  mMarginId = 1;
  mMarkerIdError = 5;
  mMarkerIdFunction = 6;
  mPluginEnabled = false;
}

CLinterPlugin::~CLinterPlugin()
{
}

void CLinterPlugin::PluginInit(HMODULE Module)
{
  __super::PluginInit(Module);
  mLintTester.Init(this, mModulePathW);
}

void CLinterPlugin::beNotified(SCNotification* Notification)
{
  __super::beNotified(Notification);

  switch (Notification->nmhdr.code)
  {
  case NPPN_READY:
    EnablePlugin();
    OnDocumentBigChange();
    break;
  case NPPN_FILEOPENED:
  case NPPN_LANGCHANGED:
  case NPPN_DOCORDERCHANGED:
  case NPPN_BUFFERACTIVATED:
    OnDocumentBigChange();
    break;
  case SCN_MODIFIED:
    if (Notification->modificationType & (SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT))
      OnDocumentSmallChange(1, false);
    break;
  case LINTER_MSG_NEW_DATA:
    if (ShowErrors(false))
      ShowErrors(false);
    break;
  default:
    //_RPT1(0, "CLinterPlugin::beNotified code:%d\r\n", Notification->nmhdr.code);
    break;
  }
}

void CLinterPlugin::EnablePlugin()
{
  HWND hScintilla = GetScintillaHandle();
  mResultListDlg.mToolbarId = GetNppMenuId(PLUGIN_MENU_SHOW_RESULT_LIST);
  mFunctionListCtrl.mToolbarId = GetNppMenuId(PLUGIN_MENU_SHOW_FUNCTION_LIST);

  ::SendMessage(hScintilla, SCI_SETMARGINTYPEN, mMarginId, SC_MARGIN_SYMBOL);

  int mask = (int) ::SendMessage(hScintilla, SCI_GETMARGINMASKN, mMarginId, 0);
  mask |= 1 << mMarkerIdError;
  mask |= 1 << mMarkerIdFunction;

  ::SendMessage(hScintilla, SCI_SETMARGINMASKN, mMarginId, mask);
  ::SendMessage(hScintilla, SCI_SETMARGINSENSITIVEN, mMarginId, 1); //boolean. click events or not

  //::SendMessage(hScintilla, SCI_MARKERDEFINE, mMarkerIdError, SC_MARK_BOOKMARK);
  //::SendMessage(hScintilla, SCI_MARKERDEFINE, mMarkerIdError, SC_MARK_UNDERLINE);
  //::SendMessage(hScintilla, SCI_MARKERSETFORE, mMarkerIdError, 0x000000);
  //::SendMessage(hScintilla, SCI_MARKERSETBACK, mMarkerIdError, 0xFF8888);
  //::SendMessage(hScintilla, SCI_MARKERSETALPHA, mMarkerIdError, 0xFF);
  ::SendMessage(hScintilla, SCI_MARKERDEFINEPIXMAP, mMarkerIdError, (LPARAM)LuaExclamation2);

  ::SendMessage(hScintilla, SCI_MARKERDEFINE, mMarkerIdFunction, SC_MARK_UNDERLINE);
  ::SendMessage(hScintilla, SCI_MARKERSETBACK, mMarkerIdFunction, 0xFF8888);

  mPluginEnabled = !mPluginEnabled;
  ::SendMessage(hScintilla, SCI_SETMOUSEDWELLTIME, 200, 0);
  if (mPluginEnabled)
  {
    OnDocumentBigChange();
  }
  else
  {
    ClearErrors();
    mLintTester.StopErrorChecking();
  }

  nlohmann::json sub;
  bool makeVisible;

  sub = mConfig["ResultWnd"];
  makeVisible = true;
  if (sub["Startup Position"].is_string())
  {
    std::string value = sub["Startup Position"];
    if (value == "Hidden")
      makeVisible = false;
    if (value == "Last" && sub["LastTimeVisible"].is_boolean() && !sub["LastTimeVisible"])
      makeVisible = false;
  }
  if (makeVisible)
    OnMenuShowResultListDlg();
  
  sub = mConfig["FunctionList"];
  makeVisible = true;
  if (sub["Startup Position"].is_string())
  {
    std::string value = sub["Startup Position"];
    if (value == "Hidden")
      makeVisible = false;
    if (value == "Last" && sub["LastTimeVisible"].is_boolean() && !sub["LastTimeVisible"])
      makeVisible = false;
  }
  if (makeVisible)
    OnMenuShowFunctionListDlg();
}

void CLinterPlugin::OnMenuCheckDocument()
{
  OnDocumentBigChange();
}

void CLinterPlugin::OnMenuShowResultListDlg()
{
  if (!mResultListDlg.isCreated())
  {
    mResultListDlg.Init((HINSTANCE)mDllHandle, mNppData.NppHandle);
    mResultListDlg.Create();
    mResultListDlg.Redraw();
    mResultListDlg.ShowWindow(true);
    return;
  }

  if (mResultListDlg.isVisible())
  {
    mResultListDlg.ShowWindow(false);
  }
  else
  {
    mResultListDlg.ShowWindow(true);
  }
}

void CLinterPlugin::OnMenuShowFunctionListDlg()
{
  if (!mFunctionListCtrl.isCreated())
  {
    mFunctionListCtrl.Init((HINSTANCE)mDllHandle, mNppData.NppHandle);
    mFunctionListCtrl.Create();
    mFunctionListCtrl.Redraw();
    mResultListDlg.ShowWindow(true);
    return;
  }
  if (mFunctionListCtrl.isVisible())
  {
    mFunctionListCtrl.ShowWindow(false);
  }
  else
  {
    mFunctionListCtrl.ShowWindow(true);
  }
}

void CLinterPlugin::OnMenuShowConfigurationDlg()
{
  if (!mConfigDlg.isCreated())
  {
    mConfigDlg.Init((HINSTANCE)mDllHandle, mNppData.NppHandle);
    mConfigDlg.DoModal();
    //mConfigDlg.Redraw();
  }
  //if (mConfigDlg.isVisible())
  //{
    //mConfigDlg.ShowWindow(false);
  //}
  //else
  //{
    //mConfigDlg.ShowWindow(true);
  //}
}

void CLinterPlugin::OnDocumentBigChange()
{
  LangType curLangType;
  SendApp(NPPM_GETCURRENTLANGTYPE, 0, (LPARAM) &curLangType);
  mLintTester.SetFileLanguage(curLangType);

  //ClearErrors();
  mResultListDlg.SetErrors(nullptr);

  OnDocumentSmallChange(0, true);
}

void CLinterPlugin::OnDocumentSmallChange(int Delay, bool ForceChanged)
{
  if (mPluginEnabled)
  {
    std::string doctxt = GetDocumentText();
    mLintTester.LintContent(doctxt, Delay, ForceChanged);
  }
}

void CLinterPlugin::FixEasyPeasy()
{
  std::string doctxt = GetDocumentText();

  std::string::size_type pos = 0u;
  while ((pos = doctxt.find(" \r", 0)) != std::string::npos)
  {
    doctxt.replace(pos, 2, "\r");
  }

  while ((pos = doctxt.find(" \n", 0)) != std::string::npos)
  {
    doctxt.replace(pos, 2, "\n");
  }

  SendEditor(SCI_SETTEXT, 0, (LPARAM)doctxt.c_str());
  OnDocumentBigChange();
  SetFocusToEditor();
}

void CLinterPlugin::FormatDocument()
{
  std::string doctxt = GetDocumentText();
  mLintTester.FormatDocument(doctxt);

  SendEditor(SCI_SETTEXT, 0, (LPARAM) doctxt.c_str());
  OnDocumentBigChange();
  SetFocusToEditor();
}

void CLinterPlugin::ClearErrors()
{
  LRESULT length = SendEditor(SCI_GETLENGTH);

  LRESULT oldid = SendEditor(SCI_GETINDICATORCURRENT);

  SendEditor(SCI_SETINDICATORCURRENT, INDICATOR_STYLE_ID_WARNING);
  SendEditor(SCI_INDICATORCLEARRANGE, 0, length);
  SendEditor(SCI_SETINDICATORCURRENT, oldid);

  SendEditor(SCI_MARKERDELETEALL, mMarkerIdError);
  //SendEditor(SCI_ANNOTATIONCLEARALL); //very slow and not needed at the moment.
}

void CLinterPlugin::ClearFunctionMarkers()
{
	SendEditor(SCI_MARKERDELETEALL, mMarkerIdFunction);
}

bool CLinterPlugin::ShowErrors(bool Force)
{
  if (mInShowErrors)
    return false;

  if (!Force && !mLintTester.GetNewErrors(mErrors)) 
    return false;

  mInShowErrors = true; //Prevent recursive class while. It's in same thread as an result of SendEditor(...

  ClearErrors();
  std::vector<int> viewList = mResultListDlg.SetErrors(&mErrors);
  mFunctionListCtrl.SetErrors(mErrors);

  LRESULT oldid = SendEditor(SCI_GETINDICATORCURRENT);
  SendEditor(SCI_INDICSETSTYLE, INDICATOR_STYLE_ID_WARNING, INDIC_BOX);  // INDIC_SQUIGGLE);
  SendEditor(SCI_INDICSETFORE, INDICATOR_STYLE_ID_WARNING, 0x0000ff);
  SendEditor(SCI_SETINDICATORCURRENT, INDICATOR_STYLE_ID_WARNING);

  
  bool showMarkers = true;
  if (mConfig["Editor"]["Error Exclamation"].is_boolean())
  {
    showMarkers = !!mConfig["Editor"]["Error Exclamation"];
  }

  for (auto& error : mErrors)
  {
    error.m_visible = false;
  }

  for (auto viewError : viewList)
  {
    mErrors[viewError].m_visible = true;
    if (showMarkers)
      SendEditor(SCI_MARKERADD, mErrors[viewError].m_line_begin - 1, mMarkerIdError);
    mErrors[viewError].m_position_begin = GetPositionFromXY(mErrors[viewError].m_line_begin, mErrors[viewError].m_column_begin);
    mErrors[viewError].m_position_end = GetPositionFromXY(mErrors[viewError].m_line_end, mErrors[viewError].m_column_end);
    SendEditor(SCI_INDICATORFILLRANGE, mErrors[viewError].m_position_begin, (mErrors[viewError].m_position_end - mErrors[viewError].m_position_begin));
  }

  SendEditor(SCI_SETINDICATORCURRENT, oldid);
  
  ShowFunctionMarkers(false);

  mInShowErrors = false;
  return true;
}

void CLinterPlugin::ShowFunctionMarkers(bool /*Force*/)
{
	ClearFunctionMarkers();
  if (mConfig["Editor"]["Function Sepeator"].is_boolean())
  {
    if (!mConfig["Editor"]["Function Sepeator"])
      return;
  }
	//SendEditor(SCI_MARKERADD, 5, mMarkerIdFunction);
  for (auto& it : mErrors)
  {
    if (it.m_severity == MRK_FUNCTION_LOCAL || it.m_severity == MRK_FUNCTION_GLOBAL)
    {
      if (it.m_line_begin != it.m_line_end)
      {
        SendEditor(SCI_MARKERADD, it.m_line_begin - 2, mMarkerIdFunction);
      }
    }
  }
}

void CLinterPlugin::OnMarginClick(int /*Modifiers*/, int64_t Position, int MarginId)
{
  if (MarginId != mMarginId) return;

  HWND hScintilla = GetScintillaHandle();
  int LineNr = (int) ::SendMessage(hScintilla, SCI_LINEFROMPOSITION, Position, 0);

  int mask = (int) ::SendMessage(hScintilla, SCI_MARKERGET, LineNr, 0);
  if ((mask & (1 << 5)) != 0)
    ::SendMessage(hScintilla, SCI_MARKERDELETE, LineNr, mMarkerIdError);
  else
    ::SendMessage(hScintilla, SCI_MARKERADD, LineNr, mMarkerIdError);
}

void CLinterPlugin::OnDoubleClick(int64_t Position, int64_t line)
{
  int64_t col = 0;
  GetXYFromPosition(Position, line, col);

  for (auto it : mErrors)
  {
    if (it.m_line_begin == line)
    {
      int64_t size = 0;
      if (it.m_severity == MRK_FUNCTION_LOCAL || it.m_severity == MRK_FUNCTION_GLOBAL)
        size = 8; //function tag
      else if (it.m_severity == MRK_IF)
        size = 2; //if tag
      else if (it.m_severity == MRK_FOR)
        size = 2; //for tag
      else if (it.m_severity == MRK_REPEAT || it.m_severity == MRK_WHILE)
        size = 5; //repeat tag

      if (col > it.m_column_begin && col <= it.m_column_begin + size)
      {
        SelectText(it.m_line_begin, it.m_column_begin, it.m_line_end, it.m_column_end, false);
        return;
      }

    }
    if (it.m_line_end == line)
    {
      int64_t size = 0;
      if (it.m_severity == MRK_FUNCTION_LOCAL || it.m_severity == MRK_FUNCTION_GLOBAL || MRK_IF || MRK_FOR || MRK_WHILE)
        size = 3; //end tag
      else if (it.m_severity == MRK_REPEAT)
        size = 5; //until tag

      if (col >= it.m_column_end - size && col < it.m_column_end)
      {
        SelectText(it.m_line_begin, it.m_column_begin, it.m_line_end, it.m_column_end, false);
        return;
      }
    }
  }
}

void CLinterPlugin::OnDwellStart(int64_t Position, int /*x*/, int /*y*/)
{
  if (Position < 0) return;

  HWND hScintilla = GetScintillaHandle();
  for (auto& error : mErrors)
  {
    if (error.m_visible && Position >= error.m_position_begin && Position <= error.m_position_end)
    {
      ::SendMessage(hScintilla, SCI_CALLTIPSHOW, Position, (LPARAM)error.m_message.c_str());
      break;
    }
  }
}

void CLinterPlugin::OnDwellEnd(int64_t /*Position*/, int /*x*/, int /*y*/)
{
  HWND hScintilla = GetScintillaHandle();
  ::SendMessage(hScintilla, SCI_CALLTIPCANCEL, 0, 0);
}



void CLinterPlugin::OnShutDown()
{
  nlohmann::json sub;

  sub = mConfig["ResultWnd"];
  sub["LastTimeVisible"] = mResultListDlg.isVisible();
  mConfig["ResultWnd"] = sub;

  sub = mConfig["FunctionList"];
  sub["LastTimeVisible"] = mFunctionListCtrl.isVisible();
  mConfig["FunctionList"] = sub;

  WritePluginConfigFile();
}