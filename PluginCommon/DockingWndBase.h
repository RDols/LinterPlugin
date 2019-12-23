#pragma once

#include "DockingWndIface.h"
#include "../notepad-plus-plus/PowerEditor/src/WinControls/DockingWnd/Docking.h"
#include "../notepad-plus-plus/PowerEditor/src/WinControls/DockingWnd/dockingResource.h"
#include <CommCtrl.h>
//#include <stdint.h>

class CDockingWndBase : public CDockingWndIface
{
public: //Constructor
  CDockingWndBase(int DialogID, const TCHAR* WindowName);
  virtual ~CDockingWndBase();

public: //Interface
  void Init(HINSTANCE hInst, HWND parent);
  virtual void Create();
  virtual void ShowWindow(bool Visible = true);
  virtual bool isCreated() { return (mHwnd != NULL); };
  virtual bool isVisible() { return mVisible; };

protected: //controls
  void SetCheck(int32_t nIDDlgItem, int32_t value);
  bool GetCheck(int32_t nIDDlgItem);

public: //statics
  BOOL ModifyStyle(DWORD dwRemove, DWORD dwAdd);
  BOOL ModifyStyleEx(DWORD dwRemove, DWORD dwAdd);

protected: //CDockingWndIface overrides
  INT_PTR OnMessage(UINT message, WPARAM wParam, LPARAM lParam) override;

protected: //Messages
  //virtual void OnInitDialog() {};
  virtual void OnControlClick(int /*ResourceId*/, HWND /*ControlWnd*/) {};
  virtual void OnContextMenu(HWND /*ControlWnd*/, int /*x*/, int /*y*/) {};
  virtual void OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/) {};
  virtual void OnShowWindow(BOOL /*Visible*/, int /*status*/);
  virtual void OnGetListDisplayInfoA(NMLVDISPINFOA* /*ListDisplayInfo*/) {};
  virtual void OnGetListDisplayInfoW(NMLVDISPINFOW* /*ListDisplayInfo*/) {};
  virtual void OnControlDoubleClick(NMITEMACTIVATE* /*ListItemActive*/) {};
  virtual void OnControlSelectionChangedA(NMTREEVIEWA* /*TreeItemActive*/) {};

protected: //Help functions

public:
  int64_t mToolbarId = 0;

protected: //Variables
  bool mVisible = false;
  int mDialogID;
  tTbData mNppDialogData;
  TCHAR* mWindowName = nullptr;
  HINSTANCE mDllHandle = 0;
  HWND mNppHandle = 0;
  HWND mHwnd = 0;
};