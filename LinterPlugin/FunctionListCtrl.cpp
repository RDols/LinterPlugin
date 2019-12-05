//
#include "stdafx.h"
#include "FunctionListCtrl.h"
#include "resource.h"
#include "LinterPlugin.h"

#include <algorithm>

static CTreeItem emptyTreeItem;

enum NColumnsErrorList
{
  CEL_LINE = 0,
  CEL_POS,
  CEL_SEVERITY,
  CEL_ERROR,
  CEL_SUBJECT,
  CEL_MESSAGE,
  CEL_COUNT
};

void CTreeItem::AddItem(const CTreeItem& lintItem)
{
  for (auto& it : mChildren)
  {
    if (lintItem.mLintItem.m_pos_begin > it.mLintItem.m_pos_end)
      continue;

    if (lintItem.mLintItem.m_pos_end < it.mLintItem.m_pos_begin)
      continue;

    if (lintItem.mLintItem.m_pos_begin > it.mLintItem.m_pos_begin)
    {
      it.AddItem(lintItem);
      return;
    }
    else if (lintItem.mLintItem.m_pos_begin < it.mLintItem.m_pos_begin)
    {
      CTreeItem copy = it;
      it = lintItem;
      it.mChildren.push_back(copy);
      return;
    }
  }
  mChildren.push_back(lintItem);
}

void CTreeItem::ExtractNamespaces()
{
  std::vector<CTreeItem> namespaces;

  //for (auto& it : mChildren)
  for (auto it = mChildren.begin(); it != mChildren.end(); /* NOTHING */)
  {
    std::replace(it->mDisplayName.begin(), it->mDisplayName.end(), ':', '.'), it->mDisplayName.end();
    int64_t sep = it->mDisplayName.find('.');
    if (sep == std::string::npos)
    {
      it++;
      continue;
    }

    std::string lhs = it->mDisplayName.substr(0, sep);
    it->mDisplayName = it->mDisplayName.substr(sep+1);

    bool NewNamespace = true;
    for (auto& it2 : namespaces)
    {
      if (it2.mDisplayName == lhs)
      {
        it2.AddItem(*it);
        NewNamespace = false;
        break;
      }
    }

    if (NewNamespace)
    {
      CTreeItem newItem;
      newItem.mLintItem.m_severity = NSeverity::MRK_NAMESPACE;
      newItem.mDisplayName = lhs;
      newItem.AddItem(*it);
      namespaces.push_back(newItem);
    }
    it = mChildren.erase(it);
  }

  mChildren.insert(mChildren.end(), namespaces.begin(), namespaces.end());

  std::sort(mChildren.begin(), mChildren.end(), [](const CTreeItem& lhs, const CTreeItem& rhs) -> bool
    {
      if (lhs.mLintItem.m_severity == rhs.mLintItem.m_severity)
        return lhs.mLintItem.m_pos_begin < rhs.mLintItem.m_pos_begin;

      if (lhs.mLintItem.m_severity == MRK_FUNCTION_LOCAL)
        return true;
      if (rhs.mLintItem.m_severity == MRK_FUNCTION_LOCAL)
        return false;
      if (lhs.mLintItem.m_severity == MRK_FUNCTION_GLOBAL)
        return true;
      return false;
    });
}

void CTreeItem::SetTree(CTreeCtrl& TreeCtrl, HTREEITEM parent)
{
  for (auto& it : mChildren)
  {
    if (it.mLintItem.m_severity == MRK_NAMESPACE)
      it.mTreeItem = TreeCtrl.InsertItem(it.mDisplayName, parent, TreeIcons::Node);
    else if (it.mLintItem.m_severity == MRK_FUNCTION_LOCAL)
      it.mTreeItem = TreeCtrl.InsertItem(it.mDisplayName, parent, TreeIcons::Leaf_Blue);
    else
      it.mTreeItem = TreeCtrl.InsertItem(it.mDisplayName, parent, TreeIcons::Leaf);

    it.SetTree(TreeCtrl, it.mTreeItem);
  }
}

const CTreeItem& CTreeItem::FindTreeData(HTREEITEM TreeItem) const
{
  for (const auto& it : mChildren)
  {
    if (it.mTreeItem == TreeItem)
      return it;

    const CTreeItem& childsearch = it.FindTreeData(TreeItem);
    if (childsearch.mTreeItem == TreeItem)
      return childsearch;
  }

  return emptyTreeItem;
}

CFunctionListCtrl::CFunctionListCtrl(CLinterPlugin* Parent)
  : CDockingWndBase(IDD_LINTER_FUNCTION_CTRL, _T("Lua Function List"))
  , mParent(Parent)
{
}

CFunctionListCtrl::~CFunctionListCtrl()
{
}

void CFunctionListCtrl::SetErrors(const std::vector<SLintError>& Errors)
{
  mTreeCtrl.DeleteAllItems();
  mFunctionRoot.mChildren.clear();

  for (auto it : Errors)
  {
    if (it.m_severity == MRK_FUNCTION_LOCAL || it.m_severity == MRK_FUNCTION_GLOBAL)
    {
      CTreeItem newItem;
      newItem.mLintItem = it;
      newItem.mDisplayName = it.m_subject;
      mFunctionRoot.AddItem(newItem);
    }
  }

  HTREEITEM root = mTreeCtrl.InsertItem("Lua Doc", TVI_ROOT, TreeIcons::Lua);
  mFunctionRoot.ExtractNamespaces();
  mFunctionRoot.SetTree(mTreeCtrl, root);
}

void CFunctionListCtrl::Create()
{
  mNppDialogData.uMask = DWS_ICONTAB;
  mNppDialogData.hIconTab = (HICON)::LoadImage(mNPP, MAKEINTRESOURCE(IDI_TAB_FUNCTIONLIST), IMAGE_ICON, 16, 16, LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);
  
  __super::Create();

  mTreeCtrl.m_hWnd = GetDlgItem(mHwnd, IDC_TREE_FUNCTIONS);
  
  CreateImageList();

  RECT clientRect;
  ::GetClientRect(mHwnd, &clientRect);
  OnSize(0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
}

void CFunctionListCtrl::CreateImageList()
{
  
  uint32_t nbBitmaps = 4;
  mTreeImageList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, nbBitmaps, 0);
  COLORREF maskColour = RGB(192, 192, 192);
  HBITMAP hbmp;  

  hbmp = ::LoadBitmap(mNPP, MAKEINTRESOURCE(IDB_BITMAP_LUA_32));
  ImageList_AddMasked(mTreeImageList, hbmp, maskColour);
  DeleteObject(hbmp);

  hbmp = ::LoadBitmap(mNPP, MAKEINTRESOURCE(IDB_BITMAP_NODE));
  ImageList_AddMasked(mTreeImageList, hbmp, maskColour);
  DeleteObject(hbmp);

  hbmp = ::LoadBitmap(mNPP, MAKEINTRESOURCE(IDB_BITMAP_LEAF));
  ImageList_AddMasked(mTreeImageList, hbmp, maskColour);
  DeleteObject(hbmp);

  hbmp = ::LoadBitmap(mNPP, MAKEINTRESOURCE(IDB_BITMAP_LEAF_BLUE));
  ImageList_AddMasked(mTreeImageList, hbmp, maskColour);
  DeleteObject(hbmp);

  TreeView_SetImageList(mTreeCtrl.m_hWnd, mTreeImageList, TVSIL_NORMAL);
}
void CFunctionListCtrl::Redraw()
{
}

void CFunctionListCtrl::OnSize(UINT /*nType*/, int cx, int cy)
{
  //mTreeCtrl.MoveWindow(0, 30, cx, cy - 30);
  mTreeCtrl.MoveWindow(0, 0, cx, cy);
}

void CFunctionListCtrl::OnControlSelectionChangedA(NMTREEVIEWA* TreeItemActive)
{
  const CTreeItem& childsearch = mFunctionRoot.FindTreeData(TreeItemActive->itemNew.hItem);
  if (childsearch.mTreeItem == TreeItemActive->itemNew.hItem)
  {
    int32_t beginLine = (int32_t) childsearch.mLintItem.m_line_begin - 1;
    int32_t beginCol = (int32_t) childsearch.mLintItem.m_column_begin - 1;
    int64_t begin = mParent->GetPositionForLine(beginLine);
    begin += mParent->utfOffset(mParent->GetLineText(beginLine), beginCol);

    int32_t endLine = (int32_t) childsearch.mLintItem.m_line_end - 1;
    int32_t endCol = (int32_t) childsearch.mLintItem.m_column_end - 1;
    int64_t end = mParent->GetPositionForLine(endLine);
    end += mParent->utfOffset(mParent->GetLineText(endLine), endCol);

    mParent->SendEditor(SCI_SETSEL, begin, end);
    mParent->SetFocusToEditor();
  }
}