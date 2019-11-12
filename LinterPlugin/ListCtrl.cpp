//
#include "stdafx.h"
#include "ListCtrl.h"

/*
A stripped non MFC version of CListCtrl.
I don't want to include MFC. 
It's a bit ugly, but i copied small parts of the MFC ClistCtrl.
*/

void CListCtrl::MoveWindow(int left, int top, int width, int height)
{
  ::MoveWindow(m_hWnd, left, top, width, height, TRUE);
}

int CListCtrl::InsertColumn(int nCol, LPCTSTR lpszColumnHeading, int nFormat,
  int nWidth, int nSubItem)
{
  LVCOLUMN column;
  column.mask = LVCF_TEXT | LVCF_FMT;
  column.pszText = (LPTSTR)lpszColumnHeading;
  column.fmt = nFormat;
  if (nWidth != -1)
  {
    column.mask |= LVCF_WIDTH;
    column.cx = nWidth;
  }
  if (nSubItem != -1)
  {
    column.mask |= LVCF_SUBITEM;
    column.iSubItem = nSubItem;
  }
  return CListCtrl::InsertColumn(nCol, &column);
}

int CListCtrl::InsertItem(UINT nMask, int nItem, LPCTSTR lpszItem, UINT nState, UINT nStateMask,
  int nImage, LPARAM lParam)
{
  LVITEM item;
  item.mask = nMask;
  item.iItem = nItem;
  item.iSubItem = 0;
  item.pszText = (LPTSTR)lpszItem;
  item.state = nState;
  item.stateMask = nStateMask;
  item.iImage = nImage;
  item.lParam = lParam;
  return CListCtrl::InsertItem(&item);
}

BOOL CListCtrl::SetItemText(int nItem, int nSubItem, LPCTSTR lpszText)
{
  LVITEM lvi;
  lvi.iSubItem = nSubItem;
  lvi.pszText = (LPTSTR)lpszText;
  return (BOOL) ::SendMessage(m_hWnd, LVM_SETITEMTEXT, nItem, (LPARAM)&lvi);
}
