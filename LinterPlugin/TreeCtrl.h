#pragma once

#include <windef.h>
#include <CommCtrl.h>

enum class TreeIcons
{
  Lua = 0,
  Node,
  Leaf,
  Leaf_Blue,
  Count
};

class CTreeCtrl
{
public: // Methods
  void MoveWindow(int top, int left, int bottom, int right);

  HTREEITEM InsertItem(const std::string& text, HTREEITEM parent, TreeIcons imageId);

public: // Messages
  BOOL DeleteAllItems()
  {
    return (BOOL) ::SendMessage(m_hWnd, TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT);
  }
  BOOL Expand(HTREEITEM hItem, UINT nCode)
  {    
    return (BOOL)::SendMessage(m_hWnd, TVM_EXPAND, nCode, (LPARAM)hItem); 
  }

public:
  HWND m_hWnd = 0;
};
