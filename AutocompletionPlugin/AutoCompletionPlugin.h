#pragma once

#include "..\PluginCommon\NppPluginBase.h"

class CAutoCompletionPlugin;
extern CAutoCompletionPlugin* Plugin;

class CAutoCompletionPlugin : public CNppPluginBase 
{
public:
	CAutoCompletionPlugin();
  virtual ~CAutoCompletionPlugin();

	public: //Interface
	public: //INppDll overrides
  virtual void PluginInit(HMODULE Module);
  virtual void beNotified(SCNotification* Notification);

public: //Menu Command
	static void EnablePluginStatic() { Plugin->EnablePlugin(); };
	void EnablePlugin();

protected: //Help functions

protected: //CNppPluginBase Overrides

protected: //Variables
  bool mPluginEnabled;
};
