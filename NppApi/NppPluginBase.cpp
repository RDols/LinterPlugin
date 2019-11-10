//
#include "stdafx.h"

#include "NppPluginBase.h"
#include "resource.h"

CNppPluginBase::CNppPluginBase()
  : mDllHandle(0)
{
  mPluginName = _T("Plugin Without Name");
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

std::string CNppPluginBase::GetLineText(int line)
{
  LRESULT length = SendEditor(SCI_LINELENGTH, line);

  char *buff = new char[length + 1];
  SendEditor(SCI_GETLINE, line, (LPARAM)buff);
  std::string text(buff, length);
  text = text.c_str();
  delete[] buff;
  return text;
}

LRESULT CNppPluginBase::GetPositionForLine(int line)
{
  return SendEditor(SCI_POSITIONFROMLINE, line);
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
    if (it->ResourceId > 0)
    {
      it->NPPMenuId = mMenuItems[count].CmdID;
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

  ::GetModuleFileName(mDllHandle, mModulePath, MAX_PATH);

  TCHAR drive[_MAX_DRIVE];
  TCHAR dir[_MAX_DIR];
  TCHAR fname[_MAX_FNAME];
  TCHAR ext[_MAX_EXT];

  _tsplitpath_s(mModulePath, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
  _tmakepath_s(mModuleFile, MAX_PATH, NULL, NULL, fname, ext);
  _tmakepath_s(mModulePath, MAX_PATH, drive, dir, NULL, NULL);
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
      SendToolbarIcons();
      break;
    case SCN_MARGINCLICK:
      OnMarginClick(Notification->modifiers, Notification->position, Notification->margin);
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
    default:
      break;
  }
}
