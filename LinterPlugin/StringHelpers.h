#pragma once

#include <string>
#include <algorithm>
#include <stdarg.h>

std::string LTrim(const std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
  std::string Trimmed = str;
  Trimmed.erase(0, Trimmed.find_first_not_of(chars));
  return Trimmed;
}

std::string RTrim(const std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
  std::string Trimmed = str;
  Trimmed.erase(Trimmed.find_last_not_of(chars) + 1);
  return Trimmed;
}

std::string Trim(const std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
  return LTrim(RTrim(str, chars), chars);
}

std::string RemoveAllWhite(const std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
  std::string s = str;
  s.erase(remove_if(s.begin(), s.end(), [&chars](const char& c) {return chars.find(c) != std::string::npos;}), s.end());
  return s;
}

std::string RemoveDoubleWhite(const std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
  std::string s = str;
  s.erase(std::unique(s.begin(), s.end(), [=](char lhs, char rhs) { return chars.find(lhs) != std::string::npos && chars.find(rhs) != std::string::npos; }), s.end());
  return s;
}

std::string ReplaceWhiteBySpace(const std::string& str, const std::string& chars = "\t\n\v\f\r", const char newChar = ' ')
{
  std::string s = str;
  std::replace_if(s.begin(), s.end(), [&chars](const char& c) {return chars.find(c) != std::string::npos; }, newChar), s.end();
  return s;

}

std::string ReplaceAllSub(const std::string& str, const std::string& search, const std::string& replace)
{
  std::string s = str;
  size_t pos = 0;
  while ((pos = s.find(search, pos)) != std::string::npos) 
  {
    s.replace(pos, search.length(), replace);
    pos += replace.length();
  }
  return s;
}


#pragma warning( push )
#pragma warning( disable : 4840)
std::string string_format(const std::string fmt, ...) 
{
  int size = ((int)fmt.size()) * 2 + 50;   // Use a rubric appropriate for your code
  std::string str;
  va_list ap;
  while (1)  // Maximum two passes on a POSIX system...
  {     
    str.resize(size);
    va_start(ap, fmt);
    int n = vsnprintf((char*)str.data(), size, fmt.c_str(), ap);
    va_end(ap);
    if (n > -1 && n < size) // Everything worked
    {  
      str.resize(n);
      return str;
    }
    if (n > -1)  // Needed size returned
      size = n + 1;   // For null char
    else
      size *= 2;      // Guess at a larger size (OS specific)
  }
  return str;
}
#pragma warning( pop ) 