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

CLinterPlugin::CLinterPlugin()
  : mResultListDlg(this)
{
  mPluginName = _T("Linter Plugin");
  //AddMenuItem(PLUGIN_MENU_SEPERATOR, _T(""), PLUGIN_MENU_SEPERATOR_FUNCTION, NULL, false, 0);
  AddMenuItem(PLUGIN_MENU_ENABLE, _T("Enable"), CLinterPlugin::EnablePluginStatic, NULL, false, 0);
  AddMenuItem(PLUGIN_MENU_CHECK_DOCUMENT, _T("Check Document"), CLinterPlugin::OnMenuCheckDocumentStatic, NULL, false, 0);
  AddMenuItem(PLUGIN_MENU_SHOW_RESULT_LIST, _T("Show Error List"), CLinterPlugin::OnMenuShowResultListDlgStatic, NULL, false, IDB_BITMAP_LUALINT);

  mMarginId = 1;
  mMarkerId = 5;
  mPluginEnabled = false;
}

CLinterPlugin::~CLinterPlugin()
{
}

void CLinterPlugin::PluginInit(HMODULE Module)
{
  __super::PluginInit(Module);
  mLintTester.Init(this, mModulePath);
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
  case SCN_UPDATEUI:
    break;
  default:
    break;
  }

  ShowErrors(false);
}

void CLinterPlugin::EnablePlugin()
{
  HWND hScintilla = GetScintillaHandle();

  ::SendMessage(hScintilla, SCI_SETMARGINTYPEN, mMarginId, SC_MARGIN_SYMBOL);

  int mask = (int) ::SendMessage(hScintilla, SCI_GETMARGINMASKN, mMarginId, 0);
  mask |= 1 << mMarkerId;
  ::SendMessage(hScintilla, SCI_SETMARGINMASKN, mMarginId, mask);
  ::SendMessage(hScintilla, SCI_SETMARGINSENSITIVEN, mMarginId, 1); //boolean. click events or not
//::SendMessage(curScintilla, SCI_MARKERDEFINE, mMarkerId, SC_MARK_CIRCLE);
  ::SendMessage(hScintilla, SCI_MARKERDEFINEPIXMAP, mMarkerId, (LPARAM)LuaExclamation2);

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

  SendApp(NPPM_SETMENUITEMCHECK, GetNppMenuId(PLUGIN_MENU_ENABLE), mPluginEnabled ? TRUE : FALSE);
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
  }

  if (mResultListDlg.isVisible())
  {
    mResultListDlg.ShowWindow(false);
    SendApp(NPPM_SETMENUITEMCHECK, GetNppMenuId(PLUGIN_MENU_SHOW_RESULT_LIST), FALSE);
  }
  else
  {
    mResultListDlg.ShowWindow(true);
    SendApp(NPPM_SETMENUITEMCHECK, GetNppMenuId(PLUGIN_MENU_SHOW_RESULT_LIST), TRUE);
  }
}

void CLinterPlugin::OnDocumentBigChange()
{
  LangType curLangType;
  SendApp(NPPM_GETCURRENTLANGTYPE, 0, (LPARAM) &curLangType);
  mLintTester.SetFileLanguage(curLangType);

  //ClearErrors();
  mResultListDlg.SetErrors(NULL);

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

  //SendEditor(SCI_SETINDICATORCURRENT, INDICATOR_STYLE_ID_HIGHLIGHT);
  //SendEditor(SCI_INDICATORCLEARRANGE, 0, length);
  //SendEditor(SCI_SETINDICATORCURRENT, oldid);

  SendEditor(SCI_MARKERDELETEALL, mMarkerId);
  //SendEditor(SCI_ANNOTATIONCLEARALL); //very slow and not needed at the moment.
}

void CLinterPlugin::ShowErrors(bool Force)
{
  if (!Force && !mLintTester.GetNewErrors(mErrors)) return;

  ClearErrors();
  std::vector<int> viewList = mResultListDlg.SetErrors(&mErrors);

  LRESULT oldid = SendEditor(SCI_GETINDICATORCURRENT);
  SendEditor(SCI_INDICSETSTYLE, INDICATOR_STYLE_ID_WARNING, INDIC_BOX);  // INDIC_SQUIGGLE);
  SendEditor(SCI_INDICSETFORE, INDICATOR_STYLE_ID_WARNING, 0x0000ff);
  SendEditor(SCI_SETINDICATORCURRENT, INDICATOR_STYLE_ID_WARNING);
  //  SendEditor(SCI_INDICSETSTYLE, INDICATOR_STYLE_ID_WARNING, INDIC_ROUNDBOX);
  //  SendEditor(SCI_INDICSETALPHA, INDICATOR_STYLE_ID_WARNING, settings.m_alpha);
  //  SendEditor(SCI_INDICSETFORE, INDICATOR_STYLE_ID_WARNING, settings.m_color);

  for (auto viewError : viewList)
  {
    SendEditor(SCI_MARKERADD, mErrors[viewError].m_line - 1, mMarkerId);
    mErrors[viewError].m_position_begin = (int)GetPositionForLine(mErrors[viewError].m_line - 1);
    mErrors[viewError].m_position_end = mErrors[viewError].m_position_begin;
    mErrors[viewError].m_position_begin += utfOffset(GetLineText(mErrors[viewError].m_line - 1), mErrors[viewError].m_column_begin - 1);
    mErrors[viewError].m_position_end += utfOffset(GetLineText(mErrors[viewError].m_line - 1), mErrors[viewError].m_column_end - 1);
    SendEditor(SCI_INDICATORFILLRANGE, mErrors[viewError].m_position_begin, (mErrors[viewError].m_position_end - mErrors[viewError].m_position_begin));
  }
/*
  for (std::vector<SLintError>::iterator it = mErrors.begin(); it != mErrors.end(); it++)
  {
    SendEditor(SCI_MARKERADD, it->m_line - 1, mMarkerId);
    it->m_position_begin = (int)GetPositionForLine(it->m_line - 1);
    it->m_position_end = it->m_position_begin;
    it->m_position_begin += utfOffset(GetLineText(it->m_line - 1), it->m_column_begin - 1);
    it->m_position_end += utfOffset(GetLineText(it->m_line - 1), it->m_column_end - 1);
    SendEditor(SCI_INDICATORFILLRANGE, it->m_position_begin, (it->m_position_end - it->m_position_begin));
  }
  */
  SendEditor(SCI_SETINDICATORCURRENT, oldid);
}

int CLinterPlugin::utfOffset(const std::string utf8, int unicodeOffset)
{
  int result = 0;
  std::string::const_iterator i = utf8.begin(), end = utf8.end();
  while (unicodeOffset > 0 && i != end)
  {
    if ((*i & 0xC0) == 0xC0 && unicodeOffset == 1)
    {
      break;
    }
    if ((*i & 0x80) == 0 || (*i & 0xC0) == 0x80)
    {
      --unicodeOffset;
    }
    if (*i != 0x0D && *i != 0x0A)
    {
      ++result;
    }
    ++i;
  }

  return result;
}


void CLinterPlugin::OnMarginClick(int Modifiers, int Position, int MarginId)
{
  if (MarginId != mMarginId) return;

  HWND hScintilla = GetScintillaHandle();
  int LineNr = (int) ::SendMessage(hScintilla, SCI_LINEFROMPOSITION, Position, 0);

  int mask = (int) ::SendMessage(hScintilla, SCI_MARKERGET, LineNr, 0);
  if ((mask & (1 << 5)) != 0)
    ::SendMessage(hScintilla, SCI_MARKERDELETE, LineNr, mMarkerId);
  else
    ::SendMessage(hScintilla, SCI_MARKERADD, LineNr, mMarkerId);
}

void CLinterPlugin::OnDwellStart(int Position, int x, int y)
{
  if (Position < 0) return;

  HWND hScintilla = GetScintillaHandle();
  for (std::vector<SLintError>::iterator it = mErrors.begin(); it != mErrors.end(); it++)
  {
    if (Position >= it->m_position_begin && Position <= it->m_position_end)
    {
      ::SendMessage(hScintilla, SCI_CALLTIPSHOW, Position, (LPARAM) it->m_message.c_str());
      break;
    }
  }
}

void CLinterPlugin::OnDwellEnd(int Position, int x, int y)
{
  HWND hScintilla = GetScintillaHandle();
  ::SendMessage(hScintilla, SCI_CALLTIPCANCEL, 0, 0);
}

