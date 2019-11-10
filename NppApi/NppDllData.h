#pragma once

#define MAX_NR_OF_ITEM_CHARS  64

#include "Notepad_plus_msgs.h"

struct SNppData
{
  HWND NppHandle;
  HWND ScintillaMainHandle;
  HWND ScintillaSecondHandle;
};

struct SShortcutKey
{
  bool Ctrl;
  bool Alt;
  bool Shift;
  UCHAR Key;
};

//typedef void(__cdecl * PFUNCSETINFO)(NppData);
typedef void(__cdecl * PFUNCPLUGINCMD)();
//typedef void(__cdecl * PBENOTIFIED)(SCNotification *);
//typedef LRESULT(__cdecl * PMESSAGEPROC)(UINT Message, WPARAM wParam, LPARAM lParam);

struct SFuncItem
{
  TCHAR ItemName[MAX_NR_OF_ITEM_CHARS];
  PFUNCPLUGINCMD CommandFunction;
  int CmdID;
  bool CheckOnInit;
  SShortcutKey* ShortcutKey;
};

struct SToolbarItem
{
  int ResourceId;
  int PluginMenuId;
  int NPPMenuId;
  HBITMAP	hToolbarBmp;
};

typedef SFuncItem * (__cdecl * PFUNCGETFUNCSARRAY)(int *);
