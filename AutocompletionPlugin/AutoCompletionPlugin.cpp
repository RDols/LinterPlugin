// AutoCompletionPlugin.cpp : Defines the exported functions for the DLL application.

#include "stdafx.h"

#include "AutoCompletionPlugin.h"
#include "resource.h"

CAutoCompletionPlugin* Plugin = new CAutoCompletionPlugin;
INppDll* nppDll = Plugin;

#define PLUGIN_MENU_ENABLE              1

CAutoCompletionPlugin::CAutoCompletionPlugin()
{
  mPluginName = L"AutoCompletion";
  mPluginShortName = "AutoCompletion";
  mPluginEnabled = false;
  AddMenuItem(PLUGIN_MENU_ENABLE, _T("Enable"), CAutoCompletionPlugin::EnablePluginStatic, NULL, true, 0);
}

CAutoCompletionPlugin ::~CAutoCompletionPlugin()
{
}

void CAutoCompletionPlugin::PluginInit(HMODULE Module)
{
  __super::PluginInit(Module);
}

void CAutoCompletionPlugin::beNotified(SCNotification* Notification)
{
  __super::beNotified(Notification);

  switch (Notification->nmhdr.code)
  {
  case NPPN_READY:
    break;
  case NPPN_FILEOPENED:
  case NPPN_LANGCHANGED:
  case NPPN_DOCORDERCHANGED:
  case NPPN_BUFFERACTIVATED:
    break;
  case SCN_MODIFIED:
    if (Notification->modificationType & (SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT))
    break;
  case SCN_UPDATEUI:
    break;
  default:
    break;
  }
  //_RPT1(0, "CAutoCompletionPlugin::beNotified code:%d\r\n", Notification->nmhdr.code);
}

void CAutoCompletionPlugin::EnablePlugin()
{
	mPluginEnabled = !mPluginEnabled;
	SendApp(NPPM_SETMENUITEMCHECK, GetNppMenuId(PLUGIN_MENU_ENABLE), mPluginEnabled ? TRUE : FALSE);
}