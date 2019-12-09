//
#include "stdafx.h"

#include "RadioGroup.h"

CRadioGroup::CRadioGroup()
{
}

CRadioGroup::~CRadioGroup()
{

}

void CRadioGroup::Create(HINSTANCE NPP, HWND Parent)
{
  mNPP = NPP;
  mParent = Parent;
}

void CRadioGroup::AddToGroup(int32_t nId, const std::string& name)
{
  mRadioButtons[nId] = name;
}

void CRadioGroup::SetDefault(const std::string& name)
{
  mDefaultId = 0;
  if (mRadioButtons.size() <= 0)
    return;

  mDefaultId = mRadioButtons.begin()->first;
  mDefaultId = NameToId(name);
}

void CRadioGroup::SetCheck(const std::string& name)
{
  SetCheck(NameToId(name));
}

void CRadioGroup::SetCheck(int32_t nId)
{
  for (auto it : mRadioButtons)
  {
    HWND hWnd = GetDlgItem(mParent, it.first);
    if (hWnd > 0)
    {
      if (it.first == nId)
        ::SendMessage(hWnd, BM_SETCHECK, TRUE, 0);
      else
        ::SendMessage(hWnd, BM_SETCHECK, FALSE, 0);
    }
  }
}

int32_t CRadioGroup::GetCheckId()
{
  for (auto it : mRadioButtons)
  {
    HWND hWnd = GetDlgItem(mParent, it.first);
    if (hWnd > 0)
    {
      int32_t value = ::SendMessage(hWnd, BM_GETCHECK, 0, 0);
      if (value == BST_CHECKED)
      {
        return it.first;
      }
    }
  }
}

std::string CRadioGroup::GetCheckName()
{
  int32_t checkedId = CRadioGroup::GetCheckId();
  return IdToName(checkedId);
}

int32_t CRadioGroup::NameToId(const std::string& name)
{
  for (auto it : mRadioButtons)
  {
    if (it.second == name)
      return it.first;

  }
  return mDefaultId;
}

std::string CRadioGroup::IdToName(int32_t nId)
{
  nId = CheckId(nId);
  for (auto it : mRadioButtons)
  {
    if (it.first == nId)
      return it.second;

  }
  return "";
}

int32_t CRadioGroup::CheckId(int32_t nId)
{
  for (auto it : mRadioButtons)
  {
    if (it.first == nId)
      return it.first;

  }
  return mDefaultId;
}