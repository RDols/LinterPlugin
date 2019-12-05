//

#include "stdafx.h"
#include "ResultListDlg.h"
#include "resource.h"
#include  "LinterPlugin.h"

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

CResultListDlg::CResultListDlg(CLinterPlugin* Parent)
  : CDockingWndBase(IDD_LINTER_RESULT_DIALOG, _T("Error list"))
  , mErrorButton(IDC_BUTTON1, IDC_ERROR_BITMAP, IDB_ERROR_ENABLED, IDB_ERROR_DISABLED)
  , mWarningButton(IDC_BUTTON2, IDC_WARNING_BITMAP, IDB_WARNING_ENABLED, IDB_WARNING_DISABLED)
  , mFormatButton(IDC_BUTTON3, IDC_FORMAT_BITMAP, IDB_FORMAT_ENABLED, IDB_FORMAT_DISABLED)
  , mIgnoreButton(IDC_BUTTON4, IDC_IGNORED_BITMAP, IDB_IGNORED_ENABLED, IDB_IGNORED_DISABLED)
  , mDebugButton(IDC_BUTTON5, IDC_DEBUG_BITMAP, IDB_DEBUG_ENABLED, IDB_DEBUG_DISABLED)
  , mFormatVisible(TRUE)
  , mInfoVisible(TRUE)
  , mParent(Parent)
{
  mErrors = &mEmptyList;

  mErrorButton.SetCheck(true);
  mWarningButton.SetCheck(true);
  mFormatButton.SetCheck(true);
  mIgnoreButton.SetCheck(false);
  mDebugButton.SetCheck(false);
}

CResultListDlg::~CResultListDlg()
{
}

void CResultListDlg::Create()
{
  __super::Create();

  mErrorButton.Create(mNPP, mHwnd);
  mWarningButton.Create(mNPP, mHwnd);
  mFormatButton.Create(mNPP, mHwnd);
  mIgnoreButton.Create(mNPP, mHwnd);
  mDebugButton.Create(mNPP, mHwnd);

  mButtonFixEasyPeasy = GetDlgItem(mHwnd, IDC_BUTTON_EASYPEASY);
  mButtonFormatDocument = GetDlgItem(mHwnd, IDC_BUTTON_FORMAT);
  mButtonRefresh = GetDlgItem(mHwnd, IDC_BUTTON_REFRESH);

  mErrorList.m_hWnd = GetDlgItem(mHwnd, IDC_LIST_ERRORS);
  ::SendMessage(mErrorList.m_hWnd, WM_NOTIFYFORMAT, (WPARAM) mParent, NF_REQUERY);
  mErrorList.SetExtendedStyle(mErrorList.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

  mErrorList.InsertColumn(CEL_LINE, _T("Line"), LVCFMT_LEFT, 60, 0);
  mErrorList.InsertColumn(CEL_POS, _T("Pos"), LVCFMT_LEFT, 60, 0);
  mErrorList.InsertColumn(CEL_SEVERITY, _T("Severity"), LVCFMT_LEFT, 80, 0);
  mErrorList.InsertColumn(CEL_ERROR, _T("Error"), LVCFMT_LEFT, 60, 0);
  mErrorList.InsertColumn(CEL_SUBJECT, _T("Subject"), LVCFMT_LEFT, 200, 0);
  mErrorList.InsertColumn(CEL_MESSAGE, _T("Message"), LVCFMT_LEFT, 500, 0);

  RECT clientRect;
  ::GetClientRect(mHwnd, &clientRect);
  OnSize(0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
  mErrorList.SetItemCountEx((int)mErrors->size(), LVSICF_NOSCROLL);
}

void CResultListDlg::OnControlClick(int /*ResourceId*/, HWND ControlWnd)
{
  bool Redraw(false);

  if (ControlWnd == mWarningButton.GetButtonHandle())
  {
    mWarningButton.Toggle();
    Redraw = true;
  }
  else if (ControlWnd == mErrorButton.GetButtonHandle())
  {
    mErrorButton.Toggle();
    Redraw = true;
  }
  else if (ControlWnd == mFormatButton.GetButtonHandle())
  {
    mFormatButton.Toggle();
    Redraw = true;
  }
  else if (ControlWnd == mIgnoreButton.GetButtonHandle())
  {
    mIgnoreButton.Toggle();
    Redraw = true;
  }
  else if (ControlWnd == mDebugButton.GetButtonHandle())
  {
    mDebugButton.Toggle();
    Redraw = true;
  }
  else if (ControlWnd == mButtonFixEasyPeasy)
    mParent->FixEasyPeasy();
  else if (ControlWnd == mButtonFormatDocument)
    mParent->FormatDocument();
  else if (ControlWnd == mButtonRefresh)
    mParent->OnDocumentBigChange();

  if (Redraw)
    mParent->ShowErrors(true);
}

void CResultListDlg::GetPupupMenu(const SLintError* ErrorItem, std::vector<stringT>& MenuItems)
{
  std::wstringstream MenuTxt;
  MenuItems.push_back(_T("Actions"));
  MenuItems.push_back(_T(""));

  MenuItems.push_back(_T("Ignore all warnings on this line"));
  if (ErrorItem->m_error_code > 110 && ErrorItem->m_error_code < 130)
  {
    MenuTxt.clear();
    MenuTxt << _T("mark global '") << TOWSTRING(ErrorItem->m_subject) << "' as ok";
    MenuItems.push_back(MenuTxt.str());
  }
}

void CResultListDlg::DoPupupMenuList(int posX, int posY)
{
  int Sel = (int) ::SendMessage(mErrorList.m_hWnd, LVM_GETSELECTIONMARK, 0, 0);
  if (Sel < 0 || Sel >= mViewList.size()) return;
  int posError = mViewList[Sel];
  if (posError < 0 || posError >= mErrors->size()) return;    
  const SLintError* ErrorItem = &mErrors->at(posError);

  std::vector<stringT> menuItems;
  GetPupupMenu(ErrorItem, menuItems);

  int Choice = DoPupupMenuList(posX, posY, menuItems);
  if (Choice == 2)
  {
    int pos = (int) mParent->SendEditor(SCI_POSITIONFROMLINE, ErrorItem->m_line_begin, 0); //be warned. I ask for the NEXT line, but m_line is zero based. Scintilla 1 based.
    pos = (int) mParent->SendEditor(SCI_POSITIONBEFORE, pos, 0);

    std::stringstream AddTxt;
    AddTxt << " -- luacheck: ignore";
    mParent->SendEditor(SCI_INSERTTEXT, pos, (LPARAM)AddTxt.str().c_str());
    mParent->OnDocumentBigChange();
    mParent->SetFocusToEditor();
  }

  if (Choice == 3)
  {
    std::stringstream AddTxt;
    AddTxt << "-- luacheck: globals " << ErrorItem->m_subject << "\r\n";
    mParent->SendEditor(SCI_INSERTTEXT, 0, (LPARAM)AddTxt.str().c_str());
    mParent->OnDocumentBigChange();
    mParent->SetFocusToEditor();
  }
}

int CResultListDlg::DoPupupMenuList(int posX, int posY, const std::vector<stringT>& MenuItems)
{
  int itemCount = (int) MenuItems.size();

  HMENU hmenu = CreatePopupMenu();
  MENUITEMINFO* menuItems = new MENUITEMINFO[itemCount];

  for (int i = 0; i < itemCount; i++)
  {
    memset(&menuItems[i], 0, sizeof(MENUITEMINFO));
    menuItems[i].cbSize = sizeof(MENUITEMINFO);
    if (MenuItems[i].size() > 0)
    {
      if (i == 0) 
      {
        menuItems[i].fMask = MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_STATE;
        menuItems[i].fType = MFT_STRING;
        menuItems[i].fState = MFS_DISABLED | MFS_DEFAULT;
      }
      else
      {
        menuItems[i].fMask = MIIM_STRING | MIIM_FTYPE | MIIM_ID;
        menuItems[i].fType = MFT_STRING;
      }
      menuItems[i].dwTypeData = (LPWSTR)MenuItems[i].c_str();
      menuItems[i].cch = (UINT) MenuItems[i].size();
    }
    else
    {
      menuItems[i].fMask = MIIM_TYPE | MIIM_ID;
      menuItems[i].fType = MFT_SEPARATOR;
    }

    menuItems[i].wID = i;
    InsertMenuItem(hmenu, i, TRUE, &menuItems[i]);
  }

  int Choice = TrackPopupMenu(hmenu, TPM_LEFTBUTTON | TPM_RETURNCMD | TPM_HORPOSANIMATION | TPM_NONOTIFY, posX-10, posY-10, 0, mErrorList.m_hWnd, 0);

  delete menuItems;
  return Choice;
}

void CResultListDlg::OnContextMenu(HWND ControlWnd, int x, int y)
{
  if (ControlWnd == mErrorList.m_hWnd)
  {
    DoPupupMenuList(x, y);
  }
}

void CResultListDlg::OnSize(UINT /*nType*/, int cx, int cy)
{
  mErrorList.MoveWindow(0, 30, cx, cy - 30);
}

void CResultListDlg::OnGetListDisplayInfoA(NMLVDISPINFOA* ListDisplayInfo)
{
  LVITEMA* pItem = &(ListDisplayInfo)->item;
  //int iItem = pItem->iItem;

  if (ListDisplayInfo->hdr.hwndFrom == mErrorList.m_hWnd)
  {
    if (pItem->mask & LVIF_TEXT && pItem->iItem < mViewList.size())
    {
      int posError = mViewList[pItem->iItem];
      if (posError < mErrors->size())
      {
        const SLintError* ErrorItem = &mErrors->at(posError);
        switch (pItem->iSubItem)
        {
          case CEL_LINE:
            _i64toa_s(ErrorItem->m_line_begin, pItem->pszText, pItem->cchTextMax, 10);
            break;
          case CEL_POS:
            _i64toa_s(ErrorItem->m_column_begin, pItem->pszText, pItem->cchTextMax, 10);
            break;
          case CEL_SEVERITY:
            switch (ErrorItem->m_severity)
            {
              case ERR_CRITICAL:
                lstrcpyA(pItem->pszText, "Critical");
                break;
              case ERR_ERROR:
                lstrcpyA(pItem->pszText, "Error");
                break;
              case ERR_WARNING:
                lstrcpyA(pItem->pszText, "Warning");
                break;
              case ERR_FORMAT:
                lstrcpyA(pItem->pszText, "Format");
                break;
              case ERR_IGNORED:
                lstrcpyA(pItem->pszText, "Ignored");
                break;
              case DBG_DEBUG:
                lstrcpyA(pItem->pszText, "Debug");
                break;
              default:
                lstrcpyA(pItem->pszText, "Unknown");
                break;
              }
            break;
          case CEL_ERROR:
            _i64toa_s(ErrorItem->m_error_code, pItem->pszText, pItem->cchTextMax, 10);
            break;
          case CEL_SUBJECT:
            lstrcpyA(pItem->pszText, ErrorItem->m_subject.c_str());
            break;
          case CEL_MESSAGE:
            lstrcpyA(pItem->pszText, ErrorItem->m_message.c_str());
            break;
          }
      }
    }
  }
}

void CResultListDlg::OnGetListDisplayInfoW(NMLVDISPINFOW* ListDisplayInfo)
{
  LVITEMW* pItem = &(ListDisplayInfo)->item;
  //int iItem = pItem->iItem;

  if (ListDisplayInfo->hdr.hwndFrom == mErrorList.m_hWnd)
  {
    if (pItem->mask & LVIF_TEXT) //valid text buffer?
    {
      const SLintError* ErrorItem = &mErrors->at(pItem->iItem);
      switch (pItem->iSubItem)
      {
        case 0:
          _i64tow_s(ErrorItem->m_line_begin, pItem->pszText, pItem->cchTextMax, 10);
          break;
        case 1:
          _i64tow_s(ErrorItem->m_column_begin, pItem->pszText, pItem->cchTextMax, 10);
          break;
      }
    }
  }
}

void CResultListDlg::OnControlDoubleClick(NMITEMACTIVATE* ListItemActive)
{
  int Sel = ListItemActive->iItem;
  if (Sel < 0 || Sel >= mViewList.size()) return;
  int posError = mViewList[Sel];
  if (posError < 0 || posError >= mErrors->size()) return;
  const SLintError* ErrorItem = &mErrors->at(posError);

/*
  #define INDICATOR_STYLE_ID_HIGHLIGHT   INDIC_CONTAINER + 3
  LRESULT oldid = mParent->SendEditor(SCI_GETINDICATORCURRENT);
  mParent->SendEditor(SCI_INDICSETSTYLE, INDICATOR_STYLE_ID_HIGHLIGHT, INDIC_BOX);  // INDIC_SQUIGGLE);
  mParent->SendEditor(SCI_INDICSETUNDER, INDICATOR_STYLE_ID_HIGHLIGHT, 0x00FFFF);
  mParent->SendEditor(SCI_SETINDICATORCURRENT, INDICATOR_STYLE_ID_HIGHLIGHT);

  mParent->SendEditor(SCI_INDICATORFILLRANGE, ErrorItem->m_position_begin, (ErrorItem->m_position_end - ErrorItem->m_position_begin));
  mParent->SendEditor(SCI_SETINDICATORCURRENT, oldid);
*/
  mParent->SendEditor(SCI_SETSEL, ErrorItem->m_position_begin, ErrorItem->m_position_end);
  mParent->SetFocusToEditor();
}

std::vector<int> CResultListDlg::SetErrors(const std::vector<SLintError>* Errors)
{
  if (Errors)
    mErrors = Errors;
  else
    mErrors = &mEmptyList;
  Redraw();
  return mViewList;
}

void CResultListDlg::Redraw()
{
  int posError = 0;
  mViewList.clear();
  //int countTotal(0);
  int countError(0);
  int countWarning(0);
  int countFormat(0);
  int countIgnored(0);
  int countDebug(0);

  for (auto Error : *mErrors)
  {
    if (Error.m_severity == ERR_CRITICAL)
    {
      if (mErrorButton.IsChecked())
        mViewList.push_back(posError);
      ++countError;
    }
    else if (Error.m_severity == ERR_ERROR)
    {
      if (mErrorButton.IsChecked())
        mViewList.push_back(posError);
      ++countError;
    }
    else if (Error.m_severity == ERR_WARNING)
    {
      if (mWarningButton.IsChecked())
        mViewList.push_back(posError);
      ++countWarning;
    }
    else if (Error.m_severity == ERR_FORMAT)
    {
      if (mFormatButton.IsChecked())
        mViewList.push_back(posError);
      ++countFormat;
    }
    else if (Error.m_severity == ERR_IGNORED)
    {
      if (mIgnoreButton.IsChecked())
        mViewList.push_back(posError);
      ++countIgnored;
    }
    else if (Error.m_severity == DBG_DEBUG)
    {
      if (mDebugButton.IsChecked())
        mViewList.push_back(posError);
      ++countDebug;
    }
    else if (Error.m_severity & ERR_MASK)
    {
      mViewList.push_back(posError);
    }

    ++posError;
  }
  
  mErrorButton.SetWindowText(countError, " Errors");
  mWarningButton.SetWindowText(countWarning, " Warnings");
  mFormatButton.SetWindowText(countFormat, " Format");
  mIgnoreButton.SetWindowText(countIgnored, " Ignored");
  mDebugButton.SetWindowText(countDebug, " Debug");

  mErrorList.SetItemCountEx((int)mViewList.size(), LVSICF_NOSCROLL);
}