//
#include "stdafx.h"
#include "DecoderBase.h"

CDecoderBase::CDecoderBase(bool IsInlineDecoder /*= false*/)
  : mIsInlineDecoder(IsInlineDecoder)
{
}

CDecoderBase::~CDecoderBase()
{
}

void CDecoderBase::Clear()
{
  mErrorsNew.clear();
  mErrors.clear();
  mErrorsHaveChanged = true;
}

bool CDecoderBase::DecodeErrors(const std::string& Output)
{
  mErrorsNew.clear();
  mErrorsHaveChanged = false;

  DocodeString(Output);

  int bufferSize = (int) Output.size()+1;
  char* buffer = new char[bufferSize];
  memcpy(buffer, Output.c_str(), bufferSize - 1);
  buffer[bufferSize - 1] = 0;

  DocodeString(buffer, bufferSize);
  delete [] buffer;

  if (mErrors.size() != mErrorsNew.size())
    mErrorsHaveChanged = true;

  mErrors = mErrorsNew;

  return mErrorsHaveChanged;
}

bool CDecoderBase::DisableInlines(const std::string& Output, std::string& DisabledOutput)
{
  int bufferSize = (int)Output.size() + 1;
  char* buffer = new char[bufferSize];
  memcpy(buffer, Output.c_str(), bufferSize - 1);
  buffer[bufferSize - 1] = 0;

  DisableInlines(buffer, bufferSize);

  DisabledOutput = buffer;

  delete [] buffer;

  return false;
}

bool CDecoderBase::IsPresent(const std::vector<SLintError>& TotalErrors, const SLintError& FindError)
{
  for (auto errorItem : TotalErrors)
  {
    if (errorItem == FindError)
      return true;
  }
  return false;
}

void CDecoderBase::AppendErrors(std::vector<SLintError>& TotalErrors)
{
  for (auto error : mErrors)
  {
    if (!IsPresent(TotalErrors, error))
    {
      if (mIsInlineDecoder)
        error.m_severity = ERR_IGNORED;
      TotalErrors.push_back(error);
    }
  }
  //TotalErrors.insert(TotalErrors.end(), mErrors.begin(), mErrors.end());
}

void CDecoderBase::AddNewError(int64_t posBegin, int64_t lineBegin, int64_t colBegin, int64_t posEnd, int64_t lineEnd, int64_t colEnd, int64_t errorCode, NSeverity severity, std::string subject, std::string message)
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
  AddNewError(newErrorEntry);
}

void CDecoderBase::AddNewError(const SLintError& NewError)
{
  mErrorsNew.push_back(NewError);
  CheckIfErrorsHaveChanged();
}

void CDecoderBase::CheckIfErrorsHaveChanged()
{
  if (mErrorsHaveChanged) return;

  if (mErrorsNew.size() > mErrors.size())
  {
    mErrorsHaveChanged = true;
    return;
  }

  int LastKey = (int) mErrorsNew.size() - 1;
  if (mErrorsNew[LastKey] != mErrors[LastKey])
  {
    mErrorsHaveChanged = true;
    return;
  }
}
