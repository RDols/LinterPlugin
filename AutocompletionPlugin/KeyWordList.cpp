//
#include "stdafx.h"

#include "KeyWordList.h"

#include <fstream>
#include <algorithm>

CKeyWordList::CKeyWordList()
{

}

CKeyWordList::~CKeyWordList()
{

}

void CKeyWordList::Clear()
{
  mKeywordList.clear();
}


int64_t FindIgnoreCase(const std::string& alltext, const std::string& search)
{
  std::string::const_iterator it = std::search(alltext.begin(), alltext.end(), search.begin(), search.end(), [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); });

  if (it != alltext.end())
    return it - alltext.begin();
  else
    return -1; // not found
}

std::string CKeyWordList::GetCompletionList(const std::string& wordStart)
{
  std::vector<std::string> ListStart;
  std::vector<std::string> ListStartIgnoreCase;
  std::vector<std::string> ListPart;
  std::vector<std::string> ListPartIgnoreCase;

  for (auto it : mKeywordList)
  {
    int64_t pos = it.find(wordStart);
    int64_t posIgnoreCase = FindIgnoreCase(it, wordStart);
    if (it == wordStart)
    {
      if (it.back() != '.' && it.back() != ':')
      {
        return "";
      }
    }
    else if (pos == 0)
      ListStart.push_back(it);
    else if (posIgnoreCase == 0)
      ListStartIgnoreCase.push_back(it);
    else if (pos != std::string::npos)
      ListPart.push_back(it);
    else if (posIgnoreCase != std::string::npos)
      ListPartIgnoreCase.push_back(it);
  }

  std::string list;
  for (auto it : mDocumentList)
    list.append(it + "|");
  for (auto it : ListStart)
    list.append(it + "|");
  for (auto it : ListStartIgnoreCase)
    list.append(it + "|");
  for (auto it : ListPart)
    list.append(it + "|");
  for (auto it : ListPartIgnoreCase)
    list.append(it + "|");

  if (list.size() > 0)
    list.pop_back();

  return list;
}

bool CKeyWordList::IsInList(const std::string& word)
{
  for (auto it : mKeywordList)
    if (it == word)
      return true;

  for (auto it : mDocumentList)
    if (it == word)
      return true;
  
  return false;
}

void CKeyWordList::Append(const std::string& filename)
{
  //std::string filename = mModulePathA;
  //filename += "Bindings.csv";
  std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
  if (file.is_open())
  {
    int64_t size = file.tellg();
    char* memblock = new char[size];
    file.seekg(0, std::ios::beg);
    file.read(memblock, size);
    file.close();

    int64_t pos = 0;
    int64_t line = 0;
    char* nameStart = nullptr;
    char* r = memblock;
    while (pos < size)
    {
      if (*r < '\r')
      {
        line++;
      }

      if (*r < 0x20)
      {
        if (nameStart && line > 2)
          AddToList(nameStart, r);

        nameStart = r + 1;
      }
      else if (nameStart)
      {
        if (*r == '.' || *r == ':')
        {
          if (nameStart && line > 2)
            AddToList(nameStart, r + 1);
        }
        else if (*r == ',')
        {
          if (nameStart && line > 2)
            AddToList(nameStart, r);

          nameStart = nullptr;
        }
      }

      pos++;
      r++;
    }

    delete[] memblock;
  }
}

void CKeyWordList::AddToList(char* nameStart, char* current)
{
  int64_t len = current - nameStart;
  if (len < 1)
    return;

  std::string name(nameStart, len);

  for (auto it : mKeywordList)
  {
    if (it == name)
      return;
  }

  mKeywordList.push_back(name);
}

void CKeyWordList::ClearDocumentList()
{
  mDocumentList.clear();
}

void CKeyWordList::AddToDocumentList(const std::string& word)
{
  if (IsInList(word))
    return;

  mDocumentList.push_back(word);
}