#pragma once

#include "DecoderBase.h"

class CLuaCheckPlainDecoder : public CDecoderBase
{
public: // Constructor
  CLuaCheckPlainDecoder(bool IsInlineDecoder = false);
  virtual ~CLuaCheckPlainDecoder();

protected: // Virtuals override from CDecoderBase
  virtual void DocodeString(char* Output, int OutputSize);
  virtual void DisableInlines(char* Output, int OutputSize);

protected: // Help functions
  void DecodeErrorsFromPlainLine(char*& readPos, const char* stringEnd);
  void DecodeErrorsFromPlainFindChar(char*& readPos, const char* stringEnd, char seperator);
  void DecodeErrorsFromPlainReplaceChar(char*& readPos, const char* stringEnd, char seperator);
  void DecodeErrorsFromPlainFindLineStart(char*& readPos, const char* stringEnd);
  void DecodeErrorsFromPlainGetSeverity(SLintError& newErrorEntry);

  void ReplaceCommentText(char*& readPos, const char* stringEnd);
};
