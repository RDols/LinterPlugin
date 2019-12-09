#pragma once

#include "..\PluginCommon\ModalWndBase.h"
#include "RadioGroup.h"

class CLinterPlugin;

class CLinterConfigDlg : public CModalWndBase
{
public:
  CLinterConfigDlg(CLinterPlugin* Parent);
  virtual ~CLinterConfigDlg();

public: // Interface

public: // CModalWndBase overrides
  BOOL OnInitDialog() override;
  void OnControlClick(int ResourceId, HWND ControlWnd) override;

protected: // Help functions
  void ReadConfig();
  void OnSave();

protected:
  CLinterPlugin* mParent;
  HWND mButtonSave = 0;
  CRadioGroup mResultStartup;
  CRadioGroup mFunctionStartup;
};

