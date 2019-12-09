#pragma once

#include "NppDll.h"
#include "../notepad-plus-plus/PowerEditor/src/MISC/PluginsManager/Notepad_plus_msgs.h"
#include "../notepad-plus-plus/PowerEditor/src/menuCmdID.h"

#include "json.hpp"

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

public:
  std::string GetLineText(int64_t line);
  void SelectText(int64_t startLine, int64_t startCol, int64_t endLine, int64_t endCol);
  LRESULT GetPositionForLine(int64_t line);
  int64_t GetPositionFromXY(int64_t line, int64_t pos);
  bool GetXYFromPosition(int64_t pos, int64_t& line, int64_t& col);
  int64_t OffsetPosition(const std::string utf8, int64_t col);
  int64_t utfOffset(const std::string utf8, int64_t unicodeOffset);

protected: //Help functions
  bool AddMenuItem(int CmdID, const TCHAR *cmdName, PFUNCPLUGINCMD pFunc, SShortcutKey *sk, bool checkOnInit, int ResourceId);
  HWND GetScintillaHandle();
  std::string GetDocumentText();
  std::wstring GetFilePart(unsigned int part);
  void SendToolbarIcons();
  int GetNppMenuId(int PluginId);

public: // Help functions
  HANDLE GetDllHandle() { return mDllHandle; };
  void ReadPluginConfigFile();
  void WritePluginConfigFile();

protected: //Virtuals
  virtual void OnMarginClick(int /*Modifiers*/, int64_t /*Position*/, int /*MarginId*/) {};
  virtual void OnDoubleClick(int64_t /*Position*/, int64_t /*line*/) {};
  virtual void OnDwellStart(int64_t /*Position*/, int /*x*/, int /*y*/) {};
  virtual void OnDwellEnd(int64_t /*Position*/, int /*x*/, int /*y*/) {};
  virtual void OnShutDown() {};

public: //INppDll overrides
  virtual void PluginInit(HMODULE Module);
  virtual void PluginCleanUp() {};
  virtual void setInfo(SNppData NppData) { mNppData = NppData; };
  virtual const TCHAR * getName() { return mPluginName.c_str(); };
  virtual SFuncItem * getFuncsArray(int* Count);
  virtual void beNotified(SCNotification* Notification);
  virtual LRESULT messageProc(UINT /*Message*/, WPARAM /*wParam*/, LPARAM /*lParam*/) { return 0; };


public:
  nlohmann::json mConfig;


protected: //Variables
  HMODULE mDllHandle;
  SNppData mNppData;
  stringT mPluginName;
  std::string mPluginShortName;
  std::string mConfigFile;
  TCHAR mModulePath[MAX_PATH];
  TCHAR mModuleFile[MAX_PATH];
  std::vector<SFuncItem> mMenuItems; //Menu items
  std::vector<SToolbarItem> mToolbarItems;
};

