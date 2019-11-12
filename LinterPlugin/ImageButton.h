#pragma once

class CImageButton
{
public: //Constructor
  CImageButton(int ButtonResourceId, int ImageResourceId, int ImageEnabledResourceId, int ImageDisabledResourceId);
  virtual ~CImageButton();

public:
  void Create(HINSTANCE NPP, HWND Parent);
  HWND GetButtonHandle() { return mButtonHandle; };
  bool IsChecked() { return mChecked; } ;
  void Toggle();
  void SetCheck(bool Checked);
  void SetWindowText(int count, const char* text);

protected:
  int mButtonResourceId;
  int mImageResourceId;
  int mImageEnabledResourceId;
  int mImageDisabledResourceId;
  bool mChecked;
  HWND mButtonHandle;
  HWND mImageHandle;
  HANDLE mImageEnabled;
  HANDLE mImageDisabled;
  HINSTANCE mNPP;
  HWND mParent;
};