#pragma once

#include "NppDll.h"
#include "Notepad_plus_msgs.h"
#include "menuCmdID.h"

#include <string>

class CNppPluginBase: public INppDll
{
public: //Constructor
  CNppPluginBase();
  virtual ~CNppPluginBase();

public:
  LRESULT SendEditor(UINT Msg, WPARAM wParam = 0, LPARAM lParam = 0);
  LRESULT SendApp(UINT Msg, WPARAM wParam = 0, LPARAM lParam = 0);
  void SetFocusToEditor();

protected: //Help functions
  bool AddMenuItem(int CmdID, const TCHAR *cmdName, PFUNCPLUGINCMD pFunc, SShortcutKey *sk, bool checkOnInit, int ResourceId);
  HWND GetScintillaHandle();
  std::string GetDocumentText();
  std::string GetLineText(int line);
  LRESULT GetPositionForLine(int line);
  std::wstring GetFilePart(unsigned int part);
  void SendToolbarIcons();
  int GetNppMenuId(int PluginId);

public: // Help functions
  HANDLE GetDllHandle() { return mDllHandle; };

protected: //Virtuals
  virtual void OnMarginClick(int Modifiers, int Position, int MarginId) {};
  virtual void OnDwellStart(int Position, int x, int y) {};
  virtual void OnDwellEnd(int Position, int x, int y) {};

public: //INppDll overrides
  virtual void PluginInit(HMODULE Module);
  virtual void PluginCleanUp() {};
  virtual void setInfo(SNppData NppData) { mNppData = NppData; };
  virtual const TCHAR * getName() { return mPluginName.c_str(); };
  virtual SFuncItem * getFuncsArray(int* Count);
  virtual void beNotified(SCNotification* Notification);
  virtual LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam) { return 0; };

protected: //Variables
  HMODULE mDllHandle;
  SNppData mNppData;
  stringT mPluginName;
  TCHAR mModulePath[MAX_PATH];
  TCHAR mModuleFile[MAX_PATH];
  std::vector<SFuncItem> mMenuItems; //Menu items
  std::vector<SToolbarItem> mToolbarItems;
};

