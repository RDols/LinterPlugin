#pragma once

#include <windef.h>
#include <CommCtrl.h>

class CListCtrl
{
public: // Methods
  void MoveWindow(int top, int left, int bottom, int right);
  int InsertColumn(_In_ int nCol, _In_z_ LPCTSTR lpszColumnHeading, _In_ int nFormat = LVCFMT_LEFT, _In_ int nWidth = -1, _In_ int nSubItem = -1);
  int InsertItem(_In_ UINT nMask, _In_ int nItem, _In_z_ LPCTSTR lpszItem, _In_ UINT nState,
    _In_ UINT nStateMask, _In_ int nImage, _In_ LPARAM lParam);
  BOOL SetItemText(_In_ int nItem, _In_ int nSubItem, _In_z_ LPCTSTR lpszText);


public: // Messages
  DWORD GetExtendedStyle() 
  {
    return (DWORD) ::SendMessage(m_hWnd, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
  }
  DWORD SetExtendedStyle(_In_ DWORD dwNewStyle)
  {
    return (DWORD) ::SendMessage(m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, (LPARAM)dwNewStyle);
  }
  int InsertColumn(_In_ int nCol, _In_ const LVCOLUMN* pColumn)
  {
    return (int) ::SendMessage(m_hWnd, LVM_INSERTCOLUMN, nCol, (LPARAM)pColumn);
  };
  int InsertItem(_In_ const LVITEM* pItem)
  {
    return (int) ::SendMessage(m_hWnd, LVM_INSERTITEM, 0, (LPARAM)pItem);
  }
  BOOL DeleteAllItems()
  {
    return (BOOL) ::SendMessage(m_hWnd, LVM_DELETEALLITEMS, 0, 0L);
  }
  BOOL SetItemCountEx(int iCount, DWORD dwFlags = LVSICF_NOINVALIDATEALL)
  {
    return (BOOL) ::SendMessage(m_hWnd, LVM_SETITEMCOUNT, (WPARAM)iCount, (LPARAM)dwFlags);
  }


public:
  HWND m_hWnd;
};
