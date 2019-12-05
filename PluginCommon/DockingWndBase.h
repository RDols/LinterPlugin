#pragma once

#include "DockingWndIface.h"
#include "../notepad-plus-plus/PowerEditor/src/WinControls/DockingWnd/Docking.h"
#include "../notepad-plus-plus/PowerEditor/src/WinControls/DockingWnd/dockingResource.h"
#include <CommCtrl.h>

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

protected: //CDockingWndIface overrides
  virtual INT_PTR OnMessage(UINT message, WPARAM wParam, LPARAM lParam);

protected: //Messages
  virtual void OnInitDialog() {};
  virtual void OnControlClick(int /*ResourceId*/, HWND /*ControlWnd*/) {};
  virtual void OnContextMenu(HWND /*ControlWnd*/, int /*x*/, int /*y*/) {};
  virtual void OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/) {};
  virtual void OnGetListDisplayInfoA(NMLVDISPINFOA* /*ListDisplayInfo*/) {};
  virtual void OnGetListDisplayInfoW(NMLVDISPINFOW* /*ListDisplayInfo*/) {};
  virtual void OnControlDoubleClick(NMITEMACTIVATE* /*ListItemActive*/) {};
  virtual void OnControlSelectionChangedA(NMTREEVIEWA* /*TreeItemActive*/) {};

protected: //Help functions

protected: //Variables
  bool mVisible;
  int mDialogID;
  tTbData mNppDialogData;
  TCHAR* mWindowName;
  HINSTANCE mNPP;
  HWND mParent;
  HWND mHwnd;

};