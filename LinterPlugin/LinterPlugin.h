#pragma once

#include "..\PluginCommon\NppPluginBase.h"
#include "LinterManager.h"
#include "ResultListDlg.h"

#include "LinterData.h"

class CLinterPlugin;
extern CLinterPlugin* Plugin;

class CLinterPlugin : public CNppPluginBase 
{
public:
  CLinterPlugin();
  virtual ~CLinterPlugin();

public: //Interface
  void ShowErrors(bool Force);
  void OnDocumentBigChange();
  void OnDocumentSmallChange(int Delay, bool ForceChanged);
  void FixEasyPeasy();
  void FormatDocument();
  static int utfOffset(const std::string utf8, int unicodeOffset);

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

protected: //Help functions
  void ClearErrors();

protected: //CNppPluginBase Overrides
  virtual void OnMarginClick(int Modifiers, int Position, int MarginId);
  virtual void OnDwellStart(int Position, int x, int y);
  virtual void OnDwellEnd(int Position, int x, int y);

protected: //Variables
  bool mPluginEnabled;
  int32_t mMarginId;
  int32_t mMarkerId;  
  std::vector<SLintError> mErrors;
  CLinterManager mLintTester;
  CResultListDlg mResultListDlg;
};
