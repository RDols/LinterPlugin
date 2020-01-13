#pragma once

enum NSeverity
{
  DBG_DEBUG = 0x1000,
  ERR_MASK = 0x2000,
  ERR_CRITICAL,
  ERR_ERROR,
  ERR_WARNING,
  ERR_FORMAT,
  ERR_IGNORED,
  MRK_MASK = 0x4000,
  MRK_FUNCTION_GLOBAL,
  MRK_FUNCTION_LOCAL,
  MRK_IF,
  MRK_FOR,
  MRK_WHILE,
  MRK_REPEAT,
  MRK_BREAK,
  MRK_TODO,
  MRK_NOTE,
  MRK_NAMESPACE,
  MRK_COMMENT_LINE,
  MRK_COMMENT_BLOCK,
  MRK_STRING_LITERAL,
  MRK_STRING_BLOCK,
  MRK_MARK
};

struct SLintError
{
  int64_t m_pos_begin;
  int64_t m_pos_end;
  int64_t m_line_begin;
  int64_t m_line_end;
  int64_t m_column_begin;
  int64_t m_column_end;
  int64_t m_position_begin;
  int64_t m_position_end;
  NSeverity m_severity;
  int64_t m_error_code;
  std::string m_message;
  std::string m_subject;
  bool m_visible;

  struct SLintError()
  {
    m_pos_begin = -1;
    m_pos_end = -1;
    m_line_begin = -1;
    m_line_end = -1;
    m_column_begin = -1;
    m_column_end = -1;
    m_position_begin = -1;
    m_position_end = -1;
    m_severity = NSeverity::DBG_DEBUG;
    m_error_code = 0;
    m_visible = false;
  }

  bool operator==(SLintError other)
  {
    return (m_line_begin == other.m_line_begin &&
      m_line_end == other.m_line_end &&
      m_column_begin == other.m_column_begin &&
      m_column_end == other.m_column_end &&
      m_error_code == other.m_error_code);
  }
  bool operator!=(SLintError other)
  {
    return !(m_line_begin == other.m_line_begin &&
      m_line_end == other.m_line_end &&
      m_column_begin == other.m_column_begin &&
      m_column_end == other.m_column_end &&
      m_error_code == other.m_error_code);
  }
  bool operator<(SLintError other)
  {
    if (m_line_begin != other.m_line_begin)
      return m_line_begin < other.m_line_begin;
    if (m_line_end != other.m_line_end)
      return m_line_end < other.m_line_end;
    if (m_column_begin != other.m_column_begin)
      return m_column_begin < other.m_column_begin;
    if (m_error_code != other.m_error_code)
      return m_error_code < other.m_error_code;
    if (m_column_end != other.m_column_end)
      return m_column_end < other.m_column_end;
    return false;
  }
};
