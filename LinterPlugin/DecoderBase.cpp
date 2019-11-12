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
  delete buffer;

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

  delete buffer;

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
        error.m_severity = SV_IGNORED;
      TotalErrors.push_back(error);
    }
  }
  //TotalErrors.insert(TotalErrors.end(), mErrors.begin(), mErrors.end());
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
