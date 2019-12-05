//
#include "stdafx.h"
#include "TreeCtrl.h"

/*
A stripped non MFC version of CListCtrl.
I don't want to include MFC.
It's a bit ugly, but i copied small parts of the MFC ClistCtrl.
*/

void CTreeCtrl::MoveWindow(int left, int top, int width, int height)
{
  ::MoveWindow(m_hWnd, left, top, width, height, TRUE);
}

HTREEITEM CTreeCtrl::InsertItem(const std::string& text, HTREEITEM parent, TreeIcons imageId)
{
  TVITEMA tvi;
  //tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
  tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE;

  tvi.state = TVIS_EXPANDED;
  tvi.stateMask = TVIS_EXPANDED;

  // Set the item label.
  tvi.pszText = (LPSTR) text.c_str();
  tvi.cchTextMax = MAX_PATH;

  // Set icon
  tvi.iImage = (int) imageId;//isNode?INDEX_CLOSED_NODE:INDEX_LEAF;
  tvi.iSelectedImage = (int) imageId;//isNode?INDEX_OPEN_NODE:INDEX_LEAF;

  // Save the full path of file in the item's application-defined data area.
  //tvi.lParam = (filePath == NULL ? 0 : reinterpret_cast<LPARAM>(new generic_string(filePath)));

  tagTVINSERTSTRUCTA tvInsertStruct;
  tvInsertStruct.item = tvi;
  tvInsertStruct.hInsertAfter = TVI_LAST;
  tvInsertStruct.hParent = parent;

  //(HTREEITEM)
  return reinterpret_cast<HTREEITEM>(::SendMessage(m_hWnd, TVM_INSERTITEMA, 0, reinterpret_cast<LPARAM>(&tvInsertStruct)));
}