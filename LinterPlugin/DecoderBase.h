#pragma once

#include "LinterData.h"

class CDecoderBase
{
public: // Constructor
  CDecoderBase(bool IsInlineDecoder = false);
  virtual ~CDecoderBase();

public: // Interface
  void Clear();
  bool DecodeErrors(const std::string& Output);
  bool DisableInlines(const std::string& Output, std::string& DisabledOutput);
  void AppendErrors(std::vector<SLintError>& TotalErrors);

public: // Interface
  bool IsPresent(const std::vector<SLintError>& TotalErrors, const SLintError& Error);

protected: // Virtuals to override
  virtual void DocodeString(char* Output, int OutputSize) {};
  virtual void DocodeString(const std::string& Output) {};
  virtual void DisableInlines(char* Output, int OutputSize) {};

protected: // Help Functions
  void AddNewError(const SLintError& NewError);
  void CheckIfErrorsHaveChanged();

protected: // Variables
  bool mIsInlineDecoder;

private: // Variables
  std::vector<SLintError> mErrors;
  std::vector<SLintError> mErrorsNew;
  bool mErrorsHaveChanged;
};