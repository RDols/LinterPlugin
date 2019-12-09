//
#include "stdafx.h"

#include "ModalWndBase.h"

CModalWndBase::CModalWndBase(int DialogID, const TCHAR* WindowName)
  : CDockingWndBase(DialogID, WindowName)
{

}

CModalWndBase::~CModalWndBase()
{

}


INT_PTR CModalWndBase::DoModal()
{
  INT_PTR responds = ::DialogBoxParam(mDllHandle, MAKEINTRESOURCE(mDialogID), mNppHandle, CModalWndBase::OnMessageStatic, reinterpret_cast<LPARAM>(this));
  return responds;
}

BOOL CModalWndBase::OnInitDialog()
{
  RECT WndRect;
  ::GetWindowRect(mHwnd, &WndRect);
  mInitial_TopLeft.x = WndRect.left;
  mInitial_TopLeft.y = WndRect.top;
  mInitial_Size.x = WndRect.right - WndRect.left;
  mInitial_Size.y = WndRect.bottom - WndRect.top;

  if (mResizeable)
  {
    ModifyStyle(DS_MODALFRAME, WS_THICKFRAME);
  }
  else
  {
    ModifyStyle(WS_THICKFRAME, DS_MODALFRAME);
  }

  return TRUE;
}
INT_PTR CModalWndBase::OnMessageStatic(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_NOTIFYFORMAT:
#ifdef UNICODE
      return NFR_UNICODE;
#else
      return NFR_ANSI;
#endif
    case WM_INITDIALOG:
    {
      CModalWndBase* dialog = (CModalWndBase*)(lParam);
      mWindowList[hwnd] = dialog;
      dialog->mHwnd = hwnd;

      return dialog->OnInitDialog();
    }
    case WM_NCDESTROY:
    {
      for (std::map<HWND, CDockingWndIface*>::iterator it = mWindowList.begin(); it != mWindowList.end(); it++)
      {
        if (it->first == hwnd)
        {
          CModalWndBase* dialog = (CModalWndBase*)(it->second);
          dialog->mHwnd = 0;
          mWindowList.erase(it);
          return 0;
        }
      }
      return TRUE;
    }
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

INT_PTR CModalWndBase::OnMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_SIZE:
    {
      if (!mResizeable)
      {
        RECT WndRect;
        ::GetWindowRect(mHwnd, &WndRect);
        int64_t width = WndRect.right - WndRect.left;
        int64_t height = WndRect.bottom - WndRect.top;
        if (width != mInitial_Size.x && height != mInitial_Size.y)
          ::SetWindowPos(mHwnd, HWND_TOPMOST, 0, 0, 600, 350, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);

        return TRUE;
      }
      break;
    }
    case WM_GETMINMAXINFO:
    {
      if (!mResizeable)
      {
        MINMAXINFO* mmi = reinterpret_cast<MINMAXINFO*>(lParam);
        //mmi->ptMaxSize.x = 600;
        //mmi->ptMaxSize.y = 350;
        mmi->ptMinTrackSize.x = mInitial_Size.x;
        mmi->ptMinTrackSize.y = mInitial_Size.y;
        mmi->ptMaxTrackSize.x = mInitial_Size.x;
        mmi->ptMaxTrackSize.y = mInitial_Size.y;
        return TRUE;
      }
      break;
    }
    case WM_CLOSE:
    {
      if (mAllowedClose)
      {
        ::EndDialog(mHwnd, IDCLOSE);
      }
      return TRUE;
    }

    case WM_COMMAND:
    {
      //int NotifyCode = HIWORD(wParam);
      int id = LOWORD(wParam);
      //HWND hWnd = (HWND)lParam;
      if (id == IDCANCEL)
      {
        if (mAllowedCancel)
        {
          ::EndDialog(mHwnd, IDCANCEL);
        }
        return TRUE;
      }
      if (id == IDOK)
      {
        if (mAllowedOK)
        {
          ::EndDialog(mHwnd, IDOK);
        }
        return TRUE;
      }
    }
  }

  return __super::OnMessage(message, wParam, lParam);
}