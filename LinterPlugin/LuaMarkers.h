#pragma once

#include "DecoderBase.h"
#include "DocWalker.h"

enum class DecodeState
{
  NORMAL = 0,
  STRING,
  STRING_BLOCK,
  COMMENT_BLOCK
};

class CLuaMarkers : public CDecoderBase
{
public: // Constructor
  CLuaMarkers();
  virtual ~CLuaMarkers();

protected: // Virtuals override from CDecoderBase
  static uint64_t LongDelimCheck(CDocWalker& doc, int64_t offset, char closingChar);
  virtual void DocodeString(char* doc, int docLength);

private:
  void Reset(char* srcDoc, int srcDocLength);
  void OnStateNormal();
  void OnStateCommentBlock();
  void OnStateStringBlock();
  void OnStateString();
  
  void PushOnStack(int64_t posBegin, int64_t lineBegin, int64_t colBegin, int64_t PosEnd, int64_t lineEnd, int64_t colEnd, int64_t errorCode, NSeverity severity, std::string subject, std::string message);
  SLintError PopFromStack();

private: //Easy access
  CDocWalker doc;
  DecodeState state = DecodeState::NORMAL;
  DecodeState statePrevious = DecodeState::NORMAL;
  CDocPosition stateStartPos;
  CDocPosition wordStart;
  CDocPosition wordEnd;
  std::vector<SLintError> statementStack;
  std::string lastWord;
  int64_t nestedBlockSizeSize = 0;
  char openedChar = 0;
  bool isNewWord = true;
  bool ignoreLindefeed = false;
};
