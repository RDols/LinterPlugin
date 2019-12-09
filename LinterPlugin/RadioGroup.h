#pragma once


class CRadioGroup
{
public:
  CRadioGroup();
  virtual ~CRadioGroup();

public: //Interface
  void Create(HINSTANCE NPP, HWND Parent);
  void AddToGroup(int32_t nId, const std::string& name);
  void SetDefault(const std::string& name);

  void SetCheck(const std::string& name);
  void SetCheck(int32_t nId);
  int32_t GetCheckId();
  std::string GetCheckName();

  int32_t NameToId(const std::string& name);
  int32_t CheckId(int32_t nId);
  std::string IdToName(int32_t nId);
  
  
protected:
  HINSTANCE mNPP = 0;
  HWND mParent = 0;
  int32_t mDefaultId = 0;
  std::map<int32_t, std::string> mRadioButtons;
};
