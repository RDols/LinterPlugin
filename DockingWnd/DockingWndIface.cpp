//
#pragma once

#include "stdafx.h"

#include "DockingWndIface.h"

std::map<HWND, CDockingWndIface*> CDockingWndIface::mWindowList;

INT_PTR CDockingWndIface::OnMessageStatic(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if (message == WM_NOTIFYFORMAT)
  {
#ifdef UNICODE
    return NFR_UNICODE;
#else
    return NFR_ANSI;
#endif
  }

  for (std::map<HWND, CDockingWndIface*>::iterator it = mWindowList.begin(); it != mWindowList.end(); it++)
  {
    if (it->first == hwnd)
    {
      return it->second->OnMessage(message, wParam, lParam);
    }
  }

  return 0;
};
