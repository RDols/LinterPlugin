//
#include "stdafx.h"
#include "LuaMarkers.h"

#include "StringHelpers.h"

CLuaMarkers::CLuaMarkers()
  : CDecoderBase(false)
{
}

CLuaMarkers::~CLuaMarkers()
{
}


void CLuaMarkers::DocodeString(char* srcDoc, int srcDocLength)
{
  Reset(srcDoc, srcDocLength);
  while (!doc.IsFinished())
  {
    if (state == DecodeState::NORMAL)
      OnStateNormal();
    else if (state == DecodeState::COMMENT_BLOCK)
      OnStateCommentBlock();
    else if (state == DecodeState::STRING_BLOCK)
      OnStateStringBlock();
    else if (state == DecodeState::STRING)
      OnStateString
     ();

    doc.Forward();
  }
}


void CLuaMarkers::Reset(char* srcDoc, int srcDocLength)
{
  doc = CDocWalker(srcDoc, srcDocLength);
  state = DecodeState::NORMAL;
  statePrevious = DecodeState::NORMAL;
  stateStartPos = doc;
  nestedBlockSizeSize = 0;
  openedChar = 0;
  isNewWord = true;
  ignoreLindefeed = false;
}


void CLuaMarkers::OnStateNormal()
{
  char c = doc.GetCurChar();
  if (c == '-')
  {
    CDocPosition endPos;
    if (doc.Match("--[[Break]]", endPos))
    {
      AddNewError(doc.mPos, doc.mLine, doc.mCol, endPos.mPos, endPos.mLine, endPos.mCol, 0, MRK_BREAK, "", "Break");
      doc.SetPosition(endPos);
      wordStart = doc;
    }
    else if (doc.Match("--[", endPos) && doc.GetCharRelative(-1) != '-')
    {
      int64_t delimSize = LongDelimCheck(doc, 3, '[');
      if (delimSize > 0)
      {
        nestedBlockSizeSize = delimSize;
        stateStartPos = doc;
        statePrevious = state;
        state = DecodeState::COMMENT_BLOCK;
        doc.SetPosition(endPos);
      }
    }
    else if (doc.Match("--", endPos))
    {
      endPos = doc.FindEOL();
      AddNewError(doc.mPos, doc.mLine, doc.mCol, endPos.mPos, endPos.mLine, endPos.mCol, 0, MRK_COMMENT_LINE, "", "Comment Line");
      doc.SetPosition(endPos);
      wordStart = doc;
    }
  }
  else if (c == '[')
  {
    int64_t delimSize = LongDelimCheck(doc, 1, '[');
    if (delimSize > 0)
    {
      nestedBlockSizeSize = delimSize;
      stateStartPos = doc;
      state = DecodeState::STRING_BLOCK;
      doc.Forward(delimSize);
    }
  }
  else if (c == '\"' || c == '\'')
  {
    openedChar = c;
    stateStartPos = doc;
    state = DecodeState::STRING;
  }
  else if (!doc.Contains(c, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890.:_"))
  {
    int64_t wordsize = doc.mPos - wordStart.mPos;
    if (wordsize > 1)
    {
      wordStart.Increase(doc);
      std::string word = doc.GetString(wordStart.mPos-1, doc.mPos-1);
      if (word == "function")
      {
        CDocPosition PosArgStart;
        CDocPosition PosArgEnd;
        CDocPosition PosArgInner;
        doc.FindWord("(", 0, PosArgStart, PosArgInner);
        doc.FindWord(")", 0, PosArgInner, PosArgEnd);
        std::string functionname = doc.GetString(doc.mPos - 1, PosArgStart.mPos - 1);
        functionname = RemoveAllWhite(functionname);
        std::string arguments = doc.GetString(PosArgStart.mPos - 1, PosArgEnd.mPos);
        arguments = ReplaceAllSub(arguments, ",", " , ");
        arguments = ReplaceAllSub(arguments, "(", " ( ");
        arguments = ReplaceAllSub(arguments, ")", " )");
        arguments = ReplaceWhiteBySpace(arguments);
        arguments = RemoveDoubleWhite(arguments);
        std::string decorated = functionname + arguments;
        if (lastWord == "local")
        {
          PushOnStack(wordStart.mPos, wordStart.mLine, wordStart.mCol, PosArgEnd.mPos, PosArgEnd.mLine, PosArgEnd.mCol, 0, MRK_FUNCTION_LOCAL, decorated, "local function");
        }
        else
        {
          PushOnStack(wordStart.mPos, wordStart.mLine, wordStart.mCol, PosArgEnd.mLine, PosArgEnd.mPos, PosArgEnd.mCol, 0, MRK_FUNCTION_GLOBAL, decorated, "global function");
        }
      }
      else if (word == "if")
      {
        CDocPosition PosArgStart;
        CDocPosition PosArgEnd;
        doc.FindWord("then", 0, PosArgStart, PosArgEnd);
        std::string statement = doc.GetString(wordStart.mPos +2, PosArgEnd.mPos - 4);
        statement = ReplaceAllSub(statement, ",", ", ");
        statement = ReplaceWhiteBySpace(statement);
        statement = RemoveDoubleWhite(statement);
        PushOnStack(wordStart.mPos, wordStart.mLine, wordStart.mCol, PosArgEnd.mPos, PosArgEnd.mLine, PosArgEnd.mCol, 0, MRK_IF, statement, "if statement");
      }
      else if (word == "for")
      {
        CDocPosition PosArgStart;
        CDocPosition PosArgEnd;
        doc.FindWord("do", 0, PosArgStart, PosArgEnd);
        std::string statement = doc.GetString(wordStart.mPos + 3, PosArgEnd.mPos - 4);
        statement = ReplaceAllSub(statement, ",", ", ");
        statement = ReplaceWhiteBySpace(statement);
        statement = RemoveDoubleWhite(statement);
        PushOnStack(wordStart.mPos, wordStart.mLine, wordStart.mCol, PosArgEnd.mPos, PosArgEnd.mLine, PosArgEnd.mCol, 0, MRK_FOR, statement, "for loop");
      }
      else if (word == "while")
      {
        CDocPosition PosArgStart;
        CDocPosition PosArgEnd;
        doc.FindWord("do", 0, PosArgStart, PosArgEnd);
        std::string statement = doc.GetString(wordStart.mPos + 5, PosArgEnd.mPos - 4);
        statement = ReplaceAllSub(statement, ",", ", ");
        statement = ReplaceWhiteBySpace(statement);
        statement = RemoveDoubleWhite(statement);
        PushOnStack(wordStart.mPos, wordStart.mLine, wordStart.mCol, PosArgEnd.mPos, PosArgEnd.mLine, PosArgEnd.mCol, 0, MRK_WHILE, statement, "while loop");
      }
      else if (word == "repeat")
      {
        PushOnStack(wordStart.mPos, wordStart.mLine, wordStart.mCol, doc.mPos, doc.mLine, doc.mCol, 0, MRK_REPEAT, "", "repeat loop");
      }
      else if (word == "end")
      {
        SLintError statement = PopFromStack();
        //statement.m_severity = ERR_ERROR;
        statement.m_pos_end = doc.mPos;
        statement.m_line_end = doc.mLine;
        statement.m_column_end = doc.mCol;
        AddNewError(statement);
      }
      else if (word == "until")
      {
        CDocPosition eolPos = doc.FindEOL();
        std::string condition = doc.GetString(wordStart.mPos + 5, eolPos.mPos);
        condition = ReplaceAllSub(condition, ",", ", ");
        condition = ReplaceWhiteBySpace(condition);
        condition = RemoveDoubleWhite(condition);

        SLintError statement = PopFromStack();
        statement.m_subject = condition;
        //statement.m_severity = ERR_ERROR;
        statement.m_pos_end = doc.mPos;
        statement.m_line_end = eolPos.mLine;
        statement.m_column_end = eolPos.mCol;
        AddNewError(statement);
      }
      lastWord = word;
    }
    wordStart = doc;
  }
}


void CLuaMarkers::OnStateCommentBlock()
{
  char c = doc.GetCurChar();
  if (c == '-')
  {
    CDocPosition endPos;
    if (doc.Match("--[", endPos) && doc.GetCharRelative(-1) != '-')
    {
      int64_t delimSize = LongDelimCheck(doc, 3, '[');
      if (delimSize > 0 && delimSize == nestedBlockSizeSize)
        AddNewError(doc.mPos, doc.mLine, doc.mCol, endPos.mPos, endPos.mLine, endPos.mCol, 2001, ERR_ERROR, "", "Nested Comment Blocks");
    }
  }
  else if (c == ']')
  {
    int64_t delimSize = LongDelimCheck(doc, 1, ']');
    if (delimSize > 0 && delimSize == nestedBlockSizeSize)
    {
      AddNewError(stateStartPos.mPos, stateStartPos.mLine, stateStartPos.mCol, doc.mPos, doc.mLine, doc.mCol + delimSize + 2, 0, MRK_COMMENT_BLOCK, "", "Comment Block");
      state = statePrevious;
    }
  }
}


void CLuaMarkers::OnStateStringBlock()
{
  char c = doc.GetCurChar();
  if (c == '[')
  {
    CDocPosition endPos;
    if (doc.Match("[", endPos))
    {
      int64_t delimSize = LongDelimCheck(doc, 1, '[');
      if (delimSize > 0 && delimSize == nestedBlockSizeSize)
        AddNewError(doc.mPos, doc.mLine, doc.mCol, endPos.mPos, endPos.mLine, endPos.mCol, 2001, ERR_ERROR, "", "Nested String Blocks");
    }
  }
  else if (c == ']')
  {
    int64_t delimSize = LongDelimCheck(doc, 1, ']');
    if (delimSize > 0 && delimSize == nestedBlockSizeSize)
    {
      AddNewError(stateStartPos.mPos, stateStartPos.mLine, stateStartPos.mCol, doc.mPos, doc.mLine, doc.mCol + delimSize + 2, 0, MRK_STRING_BLOCK, "", "String Block");
      state = DecodeState::NORMAL;
      wordStart = doc;
    }
  }
}


void CLuaMarkers::OnStateString()
{
  char c = doc.GetCurChar();
  if (c == '\\')
  {
    if (doc.GetCharRelative(1) == 'z')
      ignoreLindefeed = true;
    doc.Forward();
  }
  else if (c == openedChar)
  {
    AddNewError(stateStartPos.mPos, stateStartPos.mLine, stateStartPos.mCol, doc.mPos, doc.mLine, doc.mCol + 1, 0, MRK_STRING_LITERAL, "", "String");
    state = DecodeState::NORMAL;
    wordStart = doc;
  }
  else if (c == '\n' && !ignoreLindefeed)
  {
    AddNewError(stateStartPos.mPos, stateStartPos.mLine, stateStartPos.mCol, doc.mPos, doc.mLine, doc.mCol + 1, 0, MRK_STRING_LITERAL, "", "String (ended with eol)");
    state = DecodeState::NORMAL;
    wordStart = doc;
  }
  else if (ignoreLindefeed && !doc.Contains(c, " \r\t"))
  {
    ignoreLindefeed = false;
  }
}


// Test for [=[ ... ]=] delimiters, returns 0 if it's only a [ or ],
// return 1 for [[ or ]], returns >=2 for [=[ or ]=] and so on.
// The maximum number of '=' characters allowed is 254.
uint64_t CLuaMarkers::LongDelimCheck(CDocWalker& doc, int64_t offset, char closingChar)
{
  uint64_t sep = offset;
  while (doc.GetCharRelative(sep) == '=' && sep < 0xFF)
    sep++;
  if (doc.GetCharRelative(sep) == closingChar)
    return sep - offset + 1;
  return 0;
}

void CLuaMarkers::PushOnStack(int64_t posBegin, int64_t lineBegin, int64_t colBegin, int64_t posEnd, int64_t lineEnd, int64_t colEnd, int64_t errorCode, NSeverity severity, std::string subject, std::string message)
{
  SLintError newErrorEntry;
  newErrorEntry.m_pos_begin = posBegin;
  newErrorEntry.m_pos_end = posEnd;
  newErrorEntry.m_line_begin = lineBegin;
  newErrorEntry.m_line_end = lineEnd;
  newErrorEntry.m_column_begin = colBegin;
  newErrorEntry.m_column_end = colEnd;
  newErrorEntry.m_error_code = errorCode;
  newErrorEntry.m_severity = severity;
  newErrorEntry.m_subject = subject;
  newErrorEntry.m_message = message;
  statementStack.push_back(newErrorEntry);
}

SLintError CLuaMarkers::PopFromStack()
{
  SLintError lastEntry;
  if (statementStack.size() > 0)
  {
    lastEntry = statementStack.back();
    statementStack.pop_back();
  }
  return lastEntry;

}

