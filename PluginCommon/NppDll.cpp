//
#include "stdafx.h"
#include "NppDll.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID /*lpReserved*/)
{
  switch (ul_reason_for_call)
  {
  case DLL_PROCESS_ATTACH:
    nppDll->PluginInit(hModule);
    break;
  case DLL_THREAD_ATTACH:
    break;
  case DLL_THREAD_DETACH:
    break;
  case DLL_PROCESS_DETACH:
    nppDll->PluginCleanUp();
    break;
  }
  return TRUE;
}

extern "C" __declspec(dllexport) void setInfo(SNppData NppData) { return nppDll->setInfo(NppData); };
extern "C" __declspec(dllexport) const TCHAR * getName() { return nppDll->getName(); };
extern "C" __declspec(dllexport) SFuncItem * getFuncsArray(int* Count) { return nppDll->getFuncsArray(Count); };
extern "C" __declspec(dllexport) void beNotified(SCNotification* Notification) { nppDll->beNotified(Notification); };
extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam) { return nppDll->messageProc(Message, wParam, lParam); };

#ifdef UNICODE
extern "C" __declspec(dllexport) BOOL isUnicode() { return TRUE; };
#endif //UNICODE
