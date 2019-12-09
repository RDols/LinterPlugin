#pragma once

#include "DockingWndBase.h"

class CModalWndBase : public CDockingWndBase
{
public:
  CModalWndBase(int DialogID, const TCHAR* WindowName);
  virtual ~CModalWndBase();

public: //Interface
  virtual INT_PTR DoModal();

protected:
  virtual BOOL OnInitDialog();

public: // Callbacks
  static INT_PTR CALLBACK OnMessageStatic(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

protected: //CDockingWndIface overrides
  INT_PTR OnMessage(UINT message, WPARAM wParam, LPARAM lParam) override;

protected: //CDockingWndBase overrides
  POINT mInitial_TopLeft;
  POINT mInitial_Size;
  bool mResizeable = true;
  bool mAllowedClose = true;
  bool mAllowedOK = true;
  bool mAllowedCancel = true;
};
