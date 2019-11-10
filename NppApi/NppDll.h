#pragma once

#include "NppDllData.h"
#include "Scintilla.h"


class INppDll
{
public:
  virtual void PluginInit(HMODULE Module) = 0;
  virtual void PluginCleanUp() = 0;
  virtual void setInfo(SNppData NppData) = 0;
  virtual const TCHAR * getName() = 0;
  virtual SFuncItem * getFuncsArray(int* Count) = 0;
  virtual void beNotified(SCNotification* Notification) = 0;
  virtual LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam) = 0;
};

extern INppDll* nppDll;

