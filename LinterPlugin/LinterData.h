#pragma once

enum NSeverity
{
  SV_CRITICAL = 0,
  SV_ERROR,
  SV_WARNING,
  SV_INFO,
  SV_FORMAT,
  SV_IGNORED,
  SV_DEBUG,
  SV_COUNT
};

struct SLintError
{
  int m_line;
  int m_column_begin;
  int m_column_end;
  int m_position_begin;
  int m_position_end;
  NSeverity m_severity;
  int m_error_code;
  std::string m_message;
  std::string m_subject;

  struct SLintError()
  {
    m_line = -1;
    m_column_begin = -1;
    m_column_end = -1;
    m_position_begin = -1;
    m_position_end = -1;
    m_severity = SV_COUNT;
    m_error_code = 0;
  }

  bool operator==(SLintError other)
  {
    return (m_line == other.m_line &&
      m_column_begin == other.m_column_begin &&
      m_column_end == other.m_column_end &&
      m_error_code == other.m_error_code);
  }
  bool operator!=(SLintError other)
  {
    return !(m_line == other.m_line &&
      m_column_begin == other.m_column_begin &&
      m_column_end == other.m_column_end &&
      m_error_code == other.m_error_code);
  }
  bool operator<(SLintError other)
  {
    if (m_line != other.m_line)
      return m_line < other.m_line;
    if (m_column_begin != other.m_column_begin)
      return m_column_begin < other.m_column_begin;
    if (m_error_code != other.m_error_code)
      return m_error_code < other.m_error_code;
    if (m_column_end != other.m_column_end)
      return m_column_end < other.m_column_end;
    return false;
  }
};
