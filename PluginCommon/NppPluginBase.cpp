//
#include "stdafx.h"

#include "NppPluginBase.h"
#include "resource.h"

#include <fstream>

CNppPluginBase::CNppPluginBase()
  : mDllHandle(0)
{
  mPluginName = _T("Plugin Without Name");
  mPluginShortName = "PluginWithoutName"; //and without spaces etc
}

CNppPluginBase::~CNppPluginBase()
{
}

// TCHAR *commandName,             // the command name that you want to see in plugin menu
// PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
                                   // NULL means its a seperator
// ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
// bool checkOnInit                // optional. Make this menu item be checked visually
bool CNppPluginBase::AddMenuItem(int CmdID, const TCHAR* ItemName, PFUNCPLUGINCMD CommandFunction, SShortcutKey* ShortcutKey, bool CheckOnInit, int ResourceId)
{
  SFuncItem newMenuItem;
  lstrcpy(newMenuItem.ItemName, ItemName);  
  newMenuItem.CommandFunction = CommandFunction;
  newMenuItem.CmdID = CmdID;
  newMenuItem.CheckOnInit = CheckOnInit;
  newMenuItem.ShortcutKey = ShortcutKey;
  mMenuItems.push_back(newMenuItem);

  SToolbarItem newToolbarItem;
  memset(&newToolbarItem, 0, sizeof(SToolbarItem));

  newToolbarItem.ResourceId = ResourceId;
  newToolbarItem.PluginMenuId = CmdID;
  mToolbarItems.push_back(newToolbarItem);

  return true;
}

LRESULT CNppPluginBase::SendEditor(UINT Msg, WPARAM wParam, LPARAM lParam)
{
  HWND hScintilla = GetScintillaHandle();
  return SendMessage(hScintilla, Msg, wParam, lParam);
}

LRESULT CNppPluginBase::SendApp(UINT Msg, WPARAM wParam, LPARAM lParam)
{
  return SendMessage(mNppData.NppHandle, Msg, wParam, lParam);
}

void CNppPluginBase::SetFocusToEditor()
{
  HWND hScintilla = GetScintillaHandle();
  ::SetFocus(hScintilla);
}


HWND CNppPluginBase::GetScintillaHandle()
{
  int which = -1;
  SendApp(NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
  if (which == -1) return 0;
  return (which == 0) ? mNppData.ScintillaMainHandle : mNppData.ScintillaSecondHandle;
}

std::string CNppPluginBase::GetDocumentText()
{
  LRESULT lengthDoc = SendEditor(SCI_GETLENGTH) + 1;

  char *buff = new char[lengthDoc];
  SendEditor(SCI_GETTEXT, lengthDoc, (LPARAM)buff);
  std::string text(buff, lengthDoc);
  text = text.c_str();
  delete[] buff;
  return text;
}

std::string CNppPluginBase::GetLineText(int64_t line)
{
  LRESULT length = SendEditor(SCI_LINELENGTH, line-1);

  char *buff = new char[length + 1];
  SendEditor(SCI_GETLINE, line-1, (LPARAM)buff);
  std::string text(buff, length);
  text = text.c_str();
  delete[] buff;
  return text;
}

std::string CNppPluginBase::GetTextRange(int64_t start, int64_t end)
{
  int64_t length = end - start;
  char* buff = new char[length + 1];

  Sci_TextRange range;
  range.chrg.cpMin = (Sci_PositionCR) start;
  range.chrg.cpMax = (Sci_PositionCR) end;
  range.lpstrText = buff;
  SendEditor(SCI_GETTEXTRANGE, 0, (LPARAM)&range);

  std::string text(buff, length);
  text = text.c_str();
  delete[] buff;
  return text;
}
//line and col are ONE based
void CNppPluginBase::SelectText(int64_t startLine, int64_t startCol, int64_t endLine, int64_t endCol, bool MoveScrollbar)
{
  int64_t begin = GetPositionFromXY(startLine, startCol);
  int64_t end = GetPositionFromXY(endLine, endCol);
  SendEditor(SCI_SETSEL, begin, end);
  if (MoveScrollbar)
  {
    int64_t linesSelected = endLine - startLine + 1;
    int64_t linesInView = SendEditor(SCI_LINESONSCREEN);
    int64_t firstline = max(4, (linesInView - linesSelected) / 2);
    firstline = startLine - firstline;
    SendEditor(SCI_SETFIRSTVISIBLELINE, firstline-1);
  }
  SetFocusToEditor();
}
LRESULT CNppPluginBase::GetPositionForLine(int64_t line)
{
  return SendEditor(SCI_POSITIONFROMLINE, line-1);
}

//line and col are ONE based
int64_t CNppPluginBase::GetPositionFromXY(int64_t line, int64_t col)
{
  int64_t pos = GetPositionForLine(line);
  pos += utfOffset(GetLineText(line), col-1);
  return pos;
}

bool CNppPluginBase::GetXYFromPosition(int64_t pos, int64_t& line, int64_t& col)
{
  line = SendEditor(SCI_LINEFROMPOSITION, pos)+1;
  col = OffsetPosition(GetLineText(line), pos - GetPositionForLine(line))+1;
  return true;
}

//From position to column
int64_t CNppPluginBase::OffsetPosition(const std::string utf8, int64_t utf8col)
{
  int64_t result = 0;
  std::string::const_iterator i = utf8.begin(), end = utf8.end();
  while (utf8col > 0 && i != end)
  {
    if ((*i & 0x80) == 0 || (*i & 0xC0) == 0x80)
    {
      ++result;
    }
    if (*i != 0x0D && *i != 0x0A)
    {
      --utf8col;
    }
    ++i;
  }
  return result;
}

//From column to position
int64_t CNppPluginBase::utfOffset(const std::string utf8, int64_t unicodeOffset)
{
  int64_t result = 0;
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

std::wstring CNppPluginBase::GetFilePart(unsigned int part)
{
  LPTSTR buff = new TCHAR[MAX_PATH + 1];
  SendApp(part, MAX_PATH, (LPARAM)buff);
  std::wstring text(buff);
  delete[] buff;
  return text;
}

//note 1. hToolbarIcon is not implemented in NPP
//note 2. bitmaps only work for standard size toolbar. big and small is only for "internal" toolbars
void CNppPluginBase::SendToolbarIcons()
{
  if (mDllHandle == 0) return;

  toolbarIcons newIcon;
  newIcon.hToolbarIcon = 0;
  newIcon.hToolbarBmp = 0;
  int count(0);
  for (std::vector<SToolbarItem>::iterator it = mToolbarItems.begin(); it != mToolbarItems.end(); it++)
  {
	it->NPPMenuId = mMenuItems[count].CmdID;
	if (it->ResourceId > 0)
    {
      it->hToolbarBmp = ::LoadBitmap((HINSTANCE)mDllHandle, MAKEINTRESOURCE(it->ResourceId));
      if (it->hToolbarBmp)
      {
        newIcon.hToolbarBmp = it->hToolbarBmp;
        SendApp(NPPM_ADDTOOLBARICON, it->NPPMenuId, (LPARAM)&newIcon);
      }
    }
    ++count;
  } 
}

int CNppPluginBase::GetNppMenuId(int PluginId)
{
  for (std::vector<SToolbarItem>::iterator it = mToolbarItems.begin(); it != mToolbarItems.end(); it++)
  {
    if (it->PluginMenuId == PluginId)
      return it->NPPMenuId;
  }
  return 0;
}

void CNppPluginBase::PluginInit(HMODULE Module)
{
  mDllHandle = Module;

  //Ansi
  ::GetModuleFileNameA(mDllHandle, mModulePathA, MAX_PATH);
  char driveA[_MAX_DRIVE];
  char dirA[_MAX_DIR];
  char fnameA[_MAX_FNAME];
  char extA[_MAX_EXT];

  _splitpath_s(mModulePathA, driveA, _MAX_DRIVE, dirA, _MAX_DIR, fnameA, _MAX_FNAME, extA, _MAX_EXT);
  _makepath_s(mModuleFileA, MAX_PATH, NULL, NULL, fnameA, extA);
  _makepath_s(mModulePathA, MAX_PATH, driveA, dirA, NULL, NULL);

  //Unicode
  ::GetModuleFileNameW(mDllHandle, mModulePathW, MAX_PATH);
  WCHAR driveW[_MAX_DRIVE];
  WCHAR dirW[_MAX_DIR];
  WCHAR fnameW[_MAX_FNAME];
  WCHAR extW[_MAX_EXT];

  _wsplitpath_s(mModulePathW, driveW, _MAX_DRIVE, dirW, _MAX_DIR, fnameW, _MAX_FNAME, extW, _MAX_EXT);
  _wmakepath_s(mModuleFileW, MAX_PATH, NULL, NULL, fnameW, extW);
  _wmakepath_s(mModulePathW, MAX_PATH, driveW, dirW, NULL, NULL);
}

void CNppPluginBase::ReadPluginConfigFile()
{
  int64_t pathsize = SendApp(NPPM_GETPLUGINSCONFIGDIR);
  std::wstring ConfigPathW;
  ConfigPathW.append(pathsize+1, '\0');
  SendApp(NPPM_GETPLUGINSCONFIGDIR, (WPARAM)ConfigPathW.size(), (LPARAM)ConfigPathW.data());
  ConfigPathW.resize(pathsize);

  mConfigFile = std::string(ConfigPathW.begin(), ConfigPathW.end()) + "\\" + mPluginShortName + ".config.json";

  std::ifstream in(mConfigFile);
  if (in.is_open())
  {
    in >> mConfig;
  }
}

void CNppPluginBase::WritePluginConfigFile()
{
  std::ofstream configfile(mConfigFile);
  configfile << mConfig.dump(4) << std::endl;
}

SFuncItem* CNppPluginBase::getFuncsArray(int* Count)
{
  *Count = (int)mMenuItems.size();
  return (SFuncItem*)mMenuItems.data();
};

void CNppPluginBase::beNotified(SCNotification* Notification)
{
  switch (Notification->nmhdr.code)
  {
    case NPPN_TBMODIFICATION:
      ReadPluginConfigFile();
      SendToolbarIcons();
      break;
    case SCN_MARGINCLICK:
      OnMarginClick(Notification->modifiers, Notification->position, Notification->margin);
      break;
    case SCN_DOUBLECLICK:
      OnDoubleClick(Notification->position, Notification->line);
      break;
    case SCN_DWELLSTART:
      OnDwellStart(Notification->position, Notification->x, Notification->y);
      break;
    case SCN_DWELLEND:
      OnDwellEnd(Notification->position, Notification->x, Notification->y);
      break;
    case SCN_INDICATORCLICK:
      //Beep(1000, 100);
      break;
    case SCN_UPDATEUI:
    case SCN_PAINTED:
      //case SCN_FOCUSIN:
      //case SCN_FOCUSOUT:
      break;
    case NPPN_SHUTDOWN:
      OnShutDown();
      break;
    default:
      break;
  }
}
