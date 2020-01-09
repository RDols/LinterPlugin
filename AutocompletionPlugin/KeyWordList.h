//
#pragma once

class CKeyWordList
{
public: //Constructor
  CKeyWordList();
  virtual ~CKeyWordList();

public: //Interface
  void Clear();
  std::string GetCompletionList(const std::string& wordStart);
  bool IsInList(const std::string& word);
  void Append(const std::string& filename);
  void AddToList(char* nameStart, char* current);

  void ClearDocumentList();
  void AddToDocumentList(const std::string& word);


private: //Variables
  std::vector<std::string> mKeywordList;
  std::vector<std::string> mDocumentList;
};
