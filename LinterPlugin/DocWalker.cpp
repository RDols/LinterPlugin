#include "stdafx.h"

#include "DocWalker.h"

CDocPosition::CDocPosition()
{
  mPos = 1;
  mLine = 1;
  mCol = 1;
  mLineStart = 1;
};

void CDocPosition::Increase(const CDocContainer& doc)
{
  if (doc.GetChar(mPos) == '\n')
  {
    mPos++;
    mLine++;
    mCol = 1;
    mLineStart = mPos;
  }
  else
  {
    mPos++;
    mCol++;
  }
}

void CDocPosition::SetPosition(const CDocPosition& pos)
{
  mPos = pos.mPos;
  mLine = pos.mLine;
  mCol = pos.mCol;
  mLineStart = pos.mLineStart;
}

CDocContainer::CDocContainer(char* doc, int64_t docLength)
{
  mDoc = doc;
  mDocLength = docLength;
};

char CDocContainer::GetChar(int64_t pos) const
{
  if (pos < 1 || pos > mDocLength)
    return 0;

  return mDoc[pos - 1];
}

std::string CDocContainer::GetString(int64_t start, int64_t end)
{
  if (start < 0 || start > mDocLength)
    return "";

  int64_t len = end - start;
  if (len < 0 || end > mDocLength)
    return "";

  return std::string(mDoc + start, len);
}

CDocWalker::CDocWalker()
  : CDocContainer(nullptr, 0)
{
};



CDocWalker::CDocWalker(char* doc, int64_t docLength)
  : CDocContainer(doc, docLength)
{
};


bool CDocWalker::IsFinished()
{
  return mPos > mDocLength;
}

char CDocWalker::GetCharRelative(int64_t rel)
{
  return GetChar(mPos+rel);
}

char CDocWalker::GetCurChar()
{
  return GetChar(mPos);
}

//char CDocWalker::GetNextChar()
//{
//  return GetChar(mPos+1);
//}
//
//char CDocWalker::GetPrevChar()
//{
//  return GetChar(mPos - 1);
//}

void CDocWalker::Forward()
{
  CDocPosition::Increase(*this);
}

void CDocWalker::Forward(int64_t count)
{
  if (count < 1)
    return;

  for (int64_t i = 0; i < count; i++)
    Forward();
}


CDocPosition CDocWalker::FindEOL()
{
  CDocPosition endPosition = static_cast<CDocPosition> (*this);
  while (1)
  {
    if (endPosition.mPos > mDocLength)
      return endPosition;

    if (GetChar(endPosition.mPos) == '\n')
      return endPosition;

    endPosition.Increase(*this);
  }
}


bool CDocWalker::Match(const char* word, CDocPosition& endPosition)
{
  endPosition = static_cast<CDocPosition> (*this);

  int64_t wordSize = strlen(word);
  if (endPosition.mPos + wordSize > mDocLength)
    return false;

  for (int64_t i = 0; i < wordSize; i++)
  {
    if (GetChar(endPosition.mPos) != word[i])
      return false;

    endPosition.Increase(*this);
  }
  return true;
}


bool CDocWalker::FindWord(const char* word, int64_t LastPos, CDocPosition& startPosition, CDocPosition& endPosition)
{
  startPosition = static_cast<CDocPosition> (*this);
  endPosition = static_cast<CDocPosition> (*this);
  
  if (LastPos <= 0)
    LastPos = mDocLength;

  int64_t wordSize = strlen(word);
  LastPos = LastPos - wordSize;
  if (LastPos <= endPosition.mPos)
    return false;

  int64_t wordpos = 0;
  while (1)
  {
    char c = GetChar(endPosition.mPos);
    if (c == word[wordpos])
    {
      if (wordpos == 0)
        startPosition = endPosition;

      wordpos++;
    }
    else
      wordpos = 0;

    if (wordpos == wordSize)
      return true;

    endPosition.Increase(*this);
    if (endPosition.mPos >= LastPos)
      return false;
  }
}

bool CDocWalker::Contains(char c, const char* chars)
{
  int64_t pos = 0;
  while (1)
  {
    if (chars[pos] == '\0')
      return false;
    if (chars[pos] == c)
      return true;

    pos++;
  }
}