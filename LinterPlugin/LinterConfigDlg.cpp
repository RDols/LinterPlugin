//
#include "stdafx.h"
#include "LinterConfigDlg.h"
#include "resource.h"
#include "LinterPlugin.h"

#include <algorithm>


CLinterConfigDlg::CLinterConfigDlg(CLinterPlugin* Parent)
  : CModalWndBase(IDD_LINTER_CONFIG_DIALOG, _T("Linterplugin Configuration"))
  , mParent(Parent)
{
  mAllowedClose = true;
  mAllowedOK = false;
  mAllowedCancel = false;
  mResizeable = false;

  mResultStartup.AddToGroup(IDC_RADIO_RESULT_STARTUP_HIDDEN, "Hidden");
  mResultStartup.AddToGroup(IDC_RADIO_RESULT_STARTUP_LAST, "Last");
  mResultStartup.AddToGroup(IDC_RADIO_RESULT_STARTUP_FLOAT_CENTER, "Floating");
  mResultStartup.AddToGroup(IDC_RADIO_RESULT_STARTUP_BOTTOM, "Bottom");
  mResultStartup.SetDefault("Last");

  mFunctionStartup.AddToGroup(IDC_RADIO_FUNCTION_STARTUP_HIDDEN, "Hidden");
  mFunctionStartup.AddToGroup(IDC_RADIO_FUNCTION_STARTUP_LAST, "Last");
  mFunctionStartup.AddToGroup(IDC_RADIO_FUNCTION_STARTUP_FLOAT_CENTER, "Floating");
  mFunctionStartup.AddToGroup(IDC_RADIO_FUNCTION_STARTUP_BOTTOM, "Bottom");
  mFunctionStartup.SetDefault("Last");
}

CLinterConfigDlg::~CLinterConfigDlg()
{
}

BOOL CLinterConfigDlg::OnInitDialog()
{
  __super::OnInitDialog();


  mResultStartup.Create(mDllHandle, mHwnd);
  mFunctionStartup.Create(mDllHandle, mHwnd);

  mButtonSave = GetDlgItem(mHwnd, IDC_BUTTON_SAVE);

  ReadConfig();

  return TRUE;
}


void CLinterConfigDlg::ReadConfig()
{
  nlohmann::json sub;
  std::string value;

  sub = mParent->mConfig["ResultWnd"];
  value = "";
  if (sub["Startup Position"].is_string())
    value = sub["Startup Position"];
  mResultStartup.SetCheck(value);

  sub = mParent->mConfig["FunctionList"];
  value = "";
  if (sub["Startup Position"].is_string())
    value = sub["Startup Position"];
  mFunctionStartup.SetCheck(value);


  sub = mParent->mConfig["Editor"];
  SetCheck(IDC_CHECK_EDITOR_FUNCTION_SEPERATOR, !(sub["Function Sepeator"] == false));
  SetCheck(IDC_CHECK_EDITOR_ERROR_EXCLAMATION, !(sub["Error Exclamation"] == false));
}

void CLinterConfigDlg::OnSave()
{
  nlohmann::json sub;

  sub = mParent->mConfig["ResultWnd"];
  sub["Startup Position"] = mResultStartup.GetCheckName();
  mParent->mConfig["ResultWnd"] = sub;

  sub = mParent->mConfig["FunctionList"];
  sub["Startup Position"] = mFunctionStartup.GetCheckName();
  mParent->mConfig["FunctionList"] = sub;

  sub = mParent->mConfig["Editor"];
  sub["Function Sepeator"] = GetCheck(IDC_CHECK_EDITOR_FUNCTION_SEPERATOR);
  sub["Error Exclamation"] = GetCheck(IDC_CHECK_EDITOR_ERROR_EXCLAMATION);
  mParent->mConfig["Editor"] = sub;

  mParent->WritePluginConfigFile();
  mParent->OnDocumentBigChange();
}
void CLinterConfigDlg::OnControlClick(int /*ResourceId*/, HWND ControlWnd)
{
  if (ControlWnd == mButtonSave)
    OnSave();
}