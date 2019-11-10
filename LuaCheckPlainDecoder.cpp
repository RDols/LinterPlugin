//
#include "stdafx.h"
#include "LuaCheckPlainDecoder.h"

CLuaCheckPlainDecoder::CLuaCheckPlainDecoder(bool IsInlineDecoder /*= false*/)
  : CDecoderBase(IsInlineDecoder)
{
}

CLuaCheckPlainDecoder::~CLuaCheckPlainDecoder()
{
}

void CLuaCheckPlainDecoder::DocodeString(char* Output, int OutputSize)
{
  char* readPos = Output;
  const char* stringEnd = readPos + OutputSize;

  DecodeErrorsFromPlainLine(readPos, stringEnd);
}

void CLuaCheckPlainDecoder::DisableInlines(char* Output, int OutputSize)
{
  char* readPos = Output;
  const char* stringEnd = readPos + OutputSize;

  while (readPos <= stringEnd)
  {
    DecodeErrorsFromPlainFindChar(readPos, stringEnd, '-');
    if (*readPos == '-')
    {
      ReplaceCommentText(readPos, stringEnd);
    }
  }
}

void CLuaCheckPlainDecoder::ReplaceCommentText(char*& readPos, const char* stringEnd)
{
  while (readPos <= stringEnd)
  {
    if (*readPos == '\r')
    {
      ++readPos;
      return;
    }
    if (*readPos == 'l')
      *readPos = 'L';

    ++readPos;
  }
}

void CLuaCheckPlainDecoder::DecodeErrorsFromPlainLine(char*& readPos, const char* stringEnd)
{
  while (readPos <= stringEnd)
  {
    DecodeErrorsFromPlainFindLineStart(readPos, stringEnd);
    //const char* start = readPos;
    DecodeErrorsFromPlainReplaceChar(readPos, stringEnd, ':');
    if (*readPos == '\\' or *readPos == '/') // Was it the drive?
      DecodeErrorsFromPlainReplaceChar(readPos, stringEnd, ':');
    char* posLineNr = readPos;
    DecodeErrorsFromPlainReplaceChar(readPos, stringEnd, ':');
    char* posColumnBegin = readPos;
    DecodeErrorsFromPlainReplaceChar(readPos, stringEnd, '-');
    char* posColumnEnd = readPos;
    DecodeErrorsFromPlainReplaceChar(readPos, stringEnd, ':');
    //const char* posWhiteSpace1 = readPos;
    DecodeErrorsFromPlainReplaceChar(readPos, stringEnd, '(');
    char* posCode = readPos;
    DecodeErrorsFromPlainReplaceChar(readPos, stringEnd, ')');
    char* posInfo = readPos + 1;
    DecodeErrorsFromPlainReplaceChar(readPos, stringEnd, '\r');
    char* posLineEnd = readPos;

    char* readPos2 = posInfo;
    DecodeErrorsFromPlainFindChar(readPos2, posLineEnd, '\'');
    char* posSubStart = readPos2;
    DecodeErrorsFromPlainFindChar(readPos2, posLineEnd, '\'');
    char* posSubEnd = readPos2 - 1;

    if (readPos <= stringEnd)
    {
      SLintError newErrorEntry;
      newErrorEntry.m_line = atoi(posLineNr);
      newErrorEntry.m_column_begin = atoi(posColumnBegin);
      newErrorEntry.m_column_end = atoi(posColumnEnd) + 1;
      newErrorEntry.m_error_code = atoi(posCode + 1);
      newErrorEntry.m_message = posInfo;
      DecodeErrorsFromPlainGetSeverity(newErrorEntry);
      if (posSubStart < posSubEnd)
      {
        *posSubEnd = 0;
        newErrorEntry.m_subject = posSubStart;
      }
      AddNewError(newErrorEntry);
    }
  }
}

void CLuaCheckPlainDecoder::DecodeErrorsFromPlainFindChar(char*& readPos, const char* stringEnd, char seperator)
{
  while (readPos <= stringEnd)
  {
    if (*readPos == seperator)
    {
      ++readPos;
      return;
    }
    ++readPos;
  }
}

void CLuaCheckPlainDecoder::DecodeErrorsFromPlainReplaceChar(char*& readPos, const char* stringEnd, char seperator)
{
  while (readPos <= stringEnd)
  {
    if (*readPos == seperator)
    {
      *readPos = 0;
      ++readPos;
      return;
    }
    ++readPos;
  }
}

void CLuaCheckPlainDecoder::DecodeErrorsFromPlainFindLineStart(char*& readPos, const char* stringEnd)
{
  while (readPos <= stringEnd)
  {
    if (*readPos != '\r' && *readPos != '\n')
      return;
    ++readPos;
  }
}

void CLuaCheckPlainDecoder::DecodeErrorsFromPlainGetSeverity(SLintError& newErrorEntry)
{
  if (newErrorEntry.m_error_code < 100)
    newErrorEntry.m_severity = SV_ERROR;
  else if (newErrorEntry.m_error_code >= 600 && newErrorEntry.m_error_code < 800)
    newErrorEntry.m_severity = SV_FORMAT;
  else
    newErrorEntry.m_severity = SV_WARNING;
}
