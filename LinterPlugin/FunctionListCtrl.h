#pragma once

#include "..\PluginCommon\DockingWndBase.h"
#include "ImageButton.h"
#include "TreeCtrl.h"
#include "LinterData.h"

class CLinterPlugin;

class CTreeItem
{
public:
  std::string mDisplayName;
  SLintError mLintItem;
  std::vector<CTreeItem> mChildren;
  HTREEITEM mTreeItem = 0;

  void AddItem(const CTreeItem& lintItem);
  void ExtractNamespaces();
  void SetTree(CTreeCtrl& TreeCtrl, HTREEITEM parent);
  const CTreeItem& FindTreeData(HTREEITEM TreeItem) const;
};

class CFunctionListCtrl : public CDockingWndBase
{
public:
  CFunctionListCtrl(CLinterPlugin* Parent);
  virtual ~CFunctionListCtrl();

public: // Interface
  void SetErrors(const std::vector<SLintError>& Errors);
  void Redraw();

public: // CDockingWndBase overrides
  virtual void Create();
  virtual void OnSize(UINT nType, int cx, int cy);

protected: // Messages
  virtual void OnControlSelectionChangedA(NMTREEVIEWA* TreeItemActive);

protected: // Help functions
  void CreateImageList();

protected:
  CLinterPlugin* mParent;
  CTreeCtrl mTreeCtrl;
  HIMAGELIST mTreeImageList = 0;
  CTreeItem mFunctionRoot;
};

