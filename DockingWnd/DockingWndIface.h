#pragma once

#include <windef.h>
#include <map>

class CDockingWndIface
{
public: // Callbacks
  static INT_PTR CALLBACK OnMessageStatic(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

protected: // virtuals to override
  virtual INT_PTR OnMessage(UINT message, WPARAM wParam, LPARAM lParam) = 0;

protected: // Variables
  static std::map<HWND, CDockingWndIface*> mWindowList;
};
