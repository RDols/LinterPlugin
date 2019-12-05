#pragma once

class CDocContainer
{
public:
  CDocContainer(char* doc, int64_t docLength);
  char GetChar(int64_t pos) const; 
  std::string GetString(int64_t start, int64_t end);

public:
  int64_t mDocLength;

private:
  const char* mDoc; //reference, not owned
};

class CDocPosition
{
public:
  CDocPosition();

public:
  void Increase(const CDocContainer& doc);
  void SetPosition(const CDocPosition& pos);

public:
  int64_t mPos;
  int64_t mLine;
  int64_t mCol;
  int64_t mLineStart;
};

class CDocWalker : public CDocPosition, public CDocContainer
{
public:
  CDocWalker();
  CDocWalker(char* doc, int64_t docLength);

public:
  bool IsFinished();
  char GetCharRelative(int64_t rel);
  char GetCurChar();
  //char GetNextChar();
  //char GetPrevChar();
  void Forward();
  void Forward(int64_t count);

  CDocPosition FindEOL();
  bool Match(const char* word, CDocPosition& endPosition);
  bool FindWord(const char* word, int64_t LastPos, CDocPosition& startPosition, CDocPosition& endPosition);
  static bool Contains(char c, const char* chars);
};