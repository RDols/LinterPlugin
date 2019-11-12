//
#include "stdafx.h"
#include "ImageButton.h"
#include <sstream>

CImageButton::CImageButton(int ButtonResourceId, int ImageResourceId, int ImageEnabledResourceId, int ImageDisabledResourceId)
  : mButtonResourceId(ButtonResourceId)
  , mImageResourceId(ImageResourceId)
  , mImageEnabledResourceId(ImageEnabledResourceId)
  , mImageDisabledResourceId(ImageDisabledResourceId)
  , mChecked(true)
  , mButtonHandle(NULL)
  , mImageHandle(NULL)
  , mImageEnabled(NULL)
  , mImageDisabled(NULL)
{
}

CImageButton::~CImageButton()
{
}

void CImageButton::Create(HINSTANCE NPP, HWND Parent)
{
  mNPP = NPP;
  mParent = Parent;

  mButtonHandle = GetDlgItem(mParent, mButtonResourceId);
  mImageHandle = GetDlgItem(mParent, mImageResourceId);
  mImageEnabled = LoadImage(mNPP, MAKEINTRESOURCE(mImageEnabledResourceId), IMAGE_BITMAP, 0, 0, LR_LOADTRANSPARENT);
  mImageDisabled = LoadImage(mNPP, MAKEINTRESOURCE(mImageDisabledResourceId), IMAGE_BITMAP, 0, 0, LR_LOADTRANSPARENT);

  mChecked = !mChecked;
  Toggle();
}

void CImageButton::Toggle()
{
  mChecked = !mChecked;
  if (mChecked)
    ::SendMessage(mImageHandle, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) mImageEnabled);
  else
    ::SendMessage(mImageHandle, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) mImageDisabled);
}

void CImageButton::SetCheck(bool Checked)
{
  if (Checked == mChecked) return;

  Toggle();
}

void CImageButton::SetWindowText(int Count, const char* Text)
{
  std::stringstream WndTxt;
  WndTxt << Count << Text;
  SetWindowTextA(mButtonHandle, WndTxt.str().c_str());
}
