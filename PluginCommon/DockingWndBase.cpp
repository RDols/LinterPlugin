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

void CDockingWndBase::Init(HINSTANCE hInst, HWND parent)
{
  mNPP = hInst;
  mParent = parent;
}

void CDockingWndBase::Create()
{
  mHwnd = ::CreateDialogParam(mNPP, MAKEINTRESOURCE(mDialogID), mParent, CDockingWndIface::OnMessageStatic, reinterpret_cast<LPARAM>(this));
  mWindowList[mHwnd] = this;

  mNppDialogData.hClient = mHwnd;

  ::SendMessage(mParent, NPPM_MODELESSDIALOG, MODELESSDIALOGADD, (LPARAM)mHwnd);
  ::SendMessage(mParent, NPPM_DMMREGASDCKDLG, 0, (LPARAM) &mNppDialogData);
}

INT_PTR CDockingWndBase::OnMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_INITDIALOG:
      OnInitDialog();
      return TRUE;
      break;

    case WM_SIZE:
      OnSize((int) wParam, (int) lParam & 0xFFFF, (int) lParam >> 16);
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
      _RPT1(0, "Message %x\r\n", message);
  }

  return FALSE;
}

void CDockingWndBase::ShowWindow(bool Visible /*= true*/)
{
  mVisible = Visible;
  ::SendMessage(mParent, Visible ? NPPM_DMMSHOW : NPPM_DMMHIDE, 0, (LPARAM) mHwnd);
}
