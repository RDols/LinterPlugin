#pragma once

#include "DockingWnd\DockingWndBase.h"
#include "ImageButton.h"
#include "ListCtrl.h"
#include "LinterData.h"

class CLinterPlugin;

class CResultListDlg : public CDockingWndBase
{
public:
  CResultListDlg(CLinterPlugin* Parent);
  virtual ~CResultListDlg();

public: // Interface
  std::vector<int> SetErrors(const std::vector<SLintError>* Errors);
  void Redraw();

public: // CDockingWndBase overrides
  virtual void Create();
  virtual void OnControlClick(int ResourceId, HWND ControlWnd);
  virtual void OnContextMenu(HWND ControlWnd, int x, int y);
  virtual void OnSize(UINT nType, int cx, int cy);
  virtual void OnGetListDisplayInfoA(NMLVDISPINFOA* ListDisplayInfo);
  virtual void OnGetListDisplayInfoW(NMLVDISPINFOW* ListDisplayInfo);
  virtual void OnControlDoubleClick(NMITEMACTIVATE* ListItemActive);

protected: // Help functions
  void DoPupupMenuList(int posX, int posY);
  int DoPupupMenuList(int posX, int posY, const std::vector<stringT>& MenuItems);
  void GetPupupMenu(const SLintError* ErrorItem, std::vector<stringT>& MenuItems);

protected:
  CLinterPlugin* mParent;
  CImageButton mErrorButton;
  CImageButton mWarningButton;
  CImageButton mFormatButton;
  CImageButton mIgnoreButton;
  CImageButton mDebugButton;

  HWND mButtonFixEasyPeasy;
  HWND mButtonFormatDocument;

  //HWND mListHandle;
  CListCtrl mErrorList;
  const std::vector<SLintError>* mErrors;
  const std::vector<SLintError> mEmptyList;
  bool mFormatVisible;
  bool mInfoVisible;
  std::vector<int> mViewList;

};

