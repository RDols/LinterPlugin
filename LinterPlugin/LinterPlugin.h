#pragma once

#include "..\PluginCommon\NppPluginBase.h"
#include "LinterManager.h"
#include "ResultListDlg.h"
#include "FunctionListCtrl.h"
#include "LinterConfigDlg.h"

#include "LinterData.h"

class CLinterPlugin;
extern CLinterPlugin* Plugin;

class CLinterPlugin : public CNppPluginBase 
{
public:
  CLinterPlugin();
  virtual ~CLinterPlugin();

public: //Interface
  bool ShowErrors(bool Force);
  void ShowFunctionMarkers(bool Force);
  void OnDocumentBigChange();
  void OnDocumentSmallChange(int Delay, bool ForceChanged);
  void FixEasyPeasy();
  void FormatDocument();

public: //INppDll overrides
  virtual void PluginInit(HMODULE Module);
  virtual void beNotified(SCNotification* Notification);

public: //Menu Command
  static void EnablePluginStatic() { Plugin->EnablePlugin(); };
  void EnablePlugin();
  static void OnMenuCheckDocumentStatic() { Plugin->OnMenuCheckDocument(); };
  void OnMenuCheckDocument();
  static void OnMenuShowResultListDlgStatic() { Plugin->OnMenuShowResultListDlg(); };
  void OnMenuShowResultListDlg();
  static void OnMenuShowFunctionListDlgStatic() { Plugin->OnMenuShowFunctionListDlg(); };
  void OnMenuShowFunctionListDlg();
  static void OnMenuShowConfigutationDlgStatic() { Plugin->OnMenuShowConfigurationDlg(); };
  void OnMenuShowConfigurationDlg();

protected: //Help functions
  void ClearErrors();
  void ClearFunctionMarkers();

protected: //CNppPluginBase Overrides
  void OnMarginClick(int Modifiers, int64_t Position, int MarginId) override;
  void OnDoubleClick(int64_t Position, int64_t line) override;
  void OnDwellStart(int64_t Position, int x, int y) override;
  void OnDwellEnd(int64_t Position, int x, int y) override;
  void OnShutDown() override;
  

protected: //Variables
  bool mInShowErrors = false;
  bool mPluginEnabled;
  int32_t mMarginId;
  int32_t mMarkerIdError;
  int32_t mMarkerIdFunction;
  std::vector<SLintError> mErrors;
  CLinterManager mLintTester;
  CResultListDlg mResultListDlg;
  CFunctionListCtrl mFunctionListCtrl;
  CLinterConfigDlg mConfigDlg;
};
