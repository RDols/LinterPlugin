//
#include "stdafx.h"
#include "DockingWndBase.h"
#include "../notepad-plus-plus/PowerEditor/src/MISC/PluginsManager/Notepad_plus_msgs.h"
#include "../notepad-plus-plus/PowerEditor/src/WinControls/DockingWnd/Docking.h"
#include "../notepad-plus-plus/PowerEditor/src/WinControls/DockingWnd/dockingResource.h"
#include <CommCtrl.h>

//#include <windowsx.h>

CDockingWndBase::CDockingWndBase(int DialogID, const TCHAR* WindowName)
  : mDialogID(DialogID)
{
  int NameLength = lstrlen(WindowName)+1;
  mWindowName = new TCHAR[NameLength];
  lstrcpy(mWindowName, WindowName);

  mNppDialogData.pszName = mWindowName;
  mNppDialogData.dlgID = mDialogID;
  mNppDialogData.uMask = DWS_DF_CONT_BOTTOM;
  mNppDialogData.hIconTab = 0;
  mNppDialogData.pszAddInfo = NULL;
  //data.rcFloat
  //data.iPrevCont
  mNppDialogData.pszModuleName = mWindowName;
}

CDockingWndBase::~CDockingWndBase()
{
  for (std::map<HWND, CDockingWndIface*>::iterator it = mWindowList.begin(); it != mWindowList.end(); it++)
  {
    if (it->second == this)
    {
      mWindowList.erase(it);
    }
  }
}

//void CDockingWndBase::Init(HINSTANCE hInst, HWND parent)
void CDockingWndBase::Init(HINSTANCE dllHandle, HWND nppHandle)
{
  mDllHandle = dllHandle;
  mNppHandle = nppHandle;
}

void CDockingWndBase::Create()
{
  mHwnd = ::CreateDialogParam(mDllHandle, MAKEINTRESOURCE(mDialogID), mNppHandle, CDockingWndIface::OnMessageStatic, reinterpret_cast<LPARAM>(this));
  mWindowList[mHwnd] = this;

  mNppDialogData.hClient = mHwnd;

  ::SendMessage(mNppHandle, NPPM_MODELESSDIALOG, MODELESSDIALOGADD, (LPARAM)mHwnd);
  ::SendMessage(mNppHandle, NPPM_DMMREGASDCKDLG, 0, (LPARAM) &mNppDialogData);
}

INT_PTR CDockingWndBase::OnMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_SIZE:
      OnSize((int) wParam, (int) lParam & 0xFFFF, (int) lParam >> 16);
      return TRUE;
      break;
    case WM_SHOWWINDOW:
      OnShowWindow((BOOL) wParam, (int) lParam);
      return TRUE;
      break;
    case WM_NOTIFY:
    {
      LPNMHDR notifyHeader = (LPNMHDR)lParam;      
      if (notifyHeader->code == LVN_GETDISPINFOA)
      {
        NMLVDISPINFOA* ListDisplayInfo = reinterpret_cast<NMLVDISPINFOA*>(notifyHeader);
        OnGetListDisplayInfoA(ListDisplayInfo);
      }
      else if (notifyHeader->code == LVN_GETDISPINFOW)
      {
        NMLVDISPINFOW* ListDisplayInfo = reinterpret_cast<NMLVDISPINFOW*>(notifyHeader);
        OnGetListDisplayInfoW(ListDisplayInfo);
      }
      else if (notifyHeader->code == NM_DBLCLK)
      {
        NMITEMACTIVATE* ListItemActive = reinterpret_cast<NMITEMACTIVATE*>(notifyHeader);
        OnControlDoubleClick(ListItemActive);
      }
      else if (notifyHeader->code == TVN_SELCHANGEDA)
      {
        NMTREEVIEWA* TreeItemActive = reinterpret_cast<NMTREEVIEWA*>(notifyHeader);
        OnControlSelectionChangedA(TreeItemActive);
      }
      else
      {
        _RPT1(0, "notifyHeader->code %d %x\r\n", (int) notifyHeader->code, notifyHeader->code);
      }
      
      break;
    }

    case WM_COMMAND:
    {
      int NotifyCode = HIWORD(wParam);
      int id = LOWORD(wParam);
      HWND hWnd = (HWND) lParam;
      if (NotifyCode == BN_CLICKED)
        OnControlClick(id, hWnd);
      break;
    }
    case WM_CONTEXTMENU:
    {
      int16_t xPos = LOWORD(lParam);
      int16_t yPos = HIWORD(lParam);
      HWND hWnd = (HWND) wParam;
      OnContextMenu(hWnd, xPos, yPos);
      break;
    }

    default:
      _RPT1(0, "CDockingWndBase::OnMessage Message:%x\r\n", message);
      break;
  }

  return FALSE;
}

void CDockingWndBase::ShowWindow(bool Visible /*= true*/)
{
  mVisible = Visible;
  ::SendMessage(mNppHandle, Visible ? NPPM_DMMSHOW : NPPM_DMMHIDE, 0, (LPARAM) mHwnd);
}

void CDockingWndBase::OnShowWindow(BOOL Visible, int /*status*/)
{
  mVisible = !!Visible;
  ::SendMessage(mNppHandle, NPPM_SETMENUITEMCHECK, (WPARAM)mToolbarId, (LPARAM) Visible);
}


BOOL CDockingWndBase::ModifyStyle(DWORD dwRemove, DWORD dwAdd)
{
  DWORD dwStyle = (DWORD)::GetWindowLongPtr(mHwnd, GWL_STYLE);
  DWORD dwNewStyle = (dwStyle & ~dwRemove) | dwAdd;
  if (dwStyle == dwNewStyle)
    return FALSE;
  ::SetWindowLongPtr(mHwnd, GWL_STYLE, dwNewStyle);
  return TRUE;
}

BOOL CDockingWndBase::ModifyStyleEx(DWORD dwRemove, DWORD dwAdd)
{
  DWORD dwStyle = (DWORD)::GetWindowLongPtr(mHwnd, GWL_EXSTYLE);
  DWORD dwNewStyle = (dwStyle & ~dwRemove) | dwAdd;
  if (dwStyle == dwNewStyle)
    return FALSE;
  ::SetWindowLongPtr(mHwnd, GWL_EXSTYLE, dwNewStyle);
  return TRUE;
}

void CDockingWndBase::SetCheck(int32_t nIDDlgItem, int32_t value)
{
  HWND hWnd = GetDlgItem(mHwnd, nIDDlgItem);
  if (hWnd == 0)
    return;

  ::SendMessage(hWnd, BM_SETCHECK, value, 0);
}

bool CDockingWndBase::GetCheck(int32_t nIDDlgItem)
{
  HWND hWnd = GetDlgItem(mHwnd, nIDDlgItem);
  if (hWnd == 0)
    return false;

  int32_t value = (int32_t) ::SendMessage(hWnd, BM_GETCHECK, 0, 0);
  return value == BST_CHECKED;
}
