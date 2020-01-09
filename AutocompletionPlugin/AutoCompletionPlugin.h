#pragma once

#include "..\PluginCommon\NppPluginBase.h"
#include "KeyWordList.h"

class CAutoCompletionPlugin;
extern CAutoCompletionPlugin* Plugin;

class CAutoCompletionPlugin : public CNppPluginBase 
{
public:
	CAutoCompletionPlugin();
  virtual ~CAutoCompletionPlugin();

public: //Interface
	void OnAutoCompleteStart(bool Force);
	void OnAddChar(int ch);
	static void SendAutocompletion(PVOID lpParam, BOOLEAN TimerOrWaitFired);
	void DelayedAutocompletion(uint32_t delayms);
	void StartAutoCompletionAsync();

	public: //NppDll overrides
  virtual void PluginInit(HMODULE Module);
  virtual void beNotified(SCNotification* Notification);

public: //Menu Command
	static void EnablePluginStatic() { Plugin->EnablePlugin(); };
	void EnablePlugin();

protected: //Help functions
	void Init();
	void LoadLists();	
	bool GetCurrentWord();
	void CreateDocumentList();
	bool ReplaceCurrentWord(const char* text);

protected: //CNppPluginBase Overrides

protected: //Variables
	bool mDummy = false;
	bool mListOpen = false;
  bool mPluginEnabled = false;
	bool mInitialized = false;
	CKeyWordList mKeyWordList;

	HANDLE mDelayCheckTimer = 0;
	std::vector<std::string> mKeywordList;
	std::string mCurrentWord;
	int64_t mCurrentWordStart = 0;
	int64_t mCurrentWordEnd = 0;
};
