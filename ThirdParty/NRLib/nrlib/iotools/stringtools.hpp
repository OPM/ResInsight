// $Id: stringtools.hpp 1068 2012-09-18 11:21:53Z perroe $

// Copyright (c)  2011, Norwegian Computing Center
// All rights reserved.
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// •  Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// •  Redistributions in binary form must reproduce the above copyright notice, this list of
//    conditions and the following disclaimer in the documentation and/or other materials
//    provided with the distribution.
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
// OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef NRLIB_STRINGTOOLS_HPP
#define NRLIB_STRINGTOOLS_HPP

#include <stdlib.h> // For atoi and atof

#include <string>
#include <iomanip>
#include <vector>
#include <sstream>
#include <typeinfo>

#include "../exception/exception.hpp"

namespace NRLib {
  std::vector<std::string> GetTokens(const std::string& s);

  std::vector<std::string> GetQuotedTokens(const std::string& s);

  /// OBS: Negative values are recognized as unsigned integers on Windows.
  /// Works OK on Linux. (gcc 4.*)
  template <typename T>
  bool IsType(const std::string& s);

  /// OBS: Negative values are recognized as unsigned integers on Windows.
  /// Works OK on Linux. (gcc 4.*)
  template <typename T>
  T ParseType(const std::string& s);

  /// If used in a template function, we may get string ot string parse.
  /// Will not work with the version above, so handle special case.
  template <>
  std::string ParseType<std::string>(const std::string& s);

  /// \todo Replace precision with a format object.
  template <typename T>
  std::string ToString(const T obj, int precision=-99999);

  /// Not safe. Replaces whitespace in s with \0.
  template <typename I>
  I ParseAsciiArrayFast(std::string& s, I begin, size_t n);

  /// Get the path from a full file name.
  std::string GetPath(const std::string& filename);

  /// Get the stem of the filename (filename without path and extension)
  std::string GetStem(const std::string& filename);

  /// Get filename extension
  std::string GetExtension(const std::string& filename);

  /// Get file name only (no path) from full file name.
  std::string RemovePath(const std::string& filename);

  /// Prepend prefix to str. Returns str if prefix is empty, if str is empty
  /// or if str is a complete path starting with /.
  /// Adds '/' as directory seperator if missing.
  std::string PrependDir(const std::string& prefix,
                         const std::string& str);

  /// Replace file extension.
  std::string ReplaceExtension(const std::string& filename,
                               const std::string& extension);

  /// Add an extension to the filename.
  std::string AddExtension(const std::string& filename,
                           const std::string& extension);

  /// In string text replace all occurences odf string "out" with string "in".
  void Substitute(std::string       & text,
                  const std::string & out,
                  const std::string & in);

  /// Return uppercase of input string.
  std::string Uppercase(const std::string& text);

  bool IsNumber(const std::string & s);

  std::string Chomp(const std::string& s);

  /// String with different kinds of whitespace characters.
  inline std::string Whitespace() { return  " \t\n\r\f\v"; }

namespace NRLibPrivate {

template <class A>
class UnsafeParser
{
public:
  static A ParseType(const char* s) {
    return NRLib::ParseType<A>(s);
  }
};

template <>
class UnsafeParser<int>
{
public:
  static int ParseType(const char* s) {
    return atoi(s);
  }
};

template <>
class UnsafeParser<double>
{
public:
  static double ParseType(const char* s) {
    return atof(s);
  }
};

template <>
class UnsafeParser<float>
{
public:
  static double ParseType(const char* s) {
    return static_cast<float>(atof(s));
  }
};


} // namespace NRLibPrivate

} // namespace NRLib


/// @todo Use correct exceptions.

template <typename T>
bool NRLib::IsType(const std::string& s)
{
  std::istringstream i(s);
  T x;
  char c;
  if (!(i >> x) || i.get(c))
    return false;
  return true;
}


template <typename T>
T NRLib::ParseType(const std::string& s)
{
  std::istringstream i(s);
  T x;
  char c;
  if (!(i >> x))
    throw Exception("Failed to convert \"" + s + "\" to " + typeid(T).name());
  if (i.get(c))
    throw Exception("Could not convert whole \"" + s + "\" to " + typeid(T).name());
  return x;
}


template <typename T>
std::string NRLib::ToString(const T obj, int precision)
{
  std::ostringstream o;
  if (precision!=-99999) {
    o << std::fixed << std::setprecision(precision);
  }
  if (!(o << obj)) {
    throw Exception("Bad conversion.");
  }
  return o.str();
}


template <typename I>
I NRLib::ParseAsciiArrayFast(std::string& s, I begin, size_t n)
{
  typedef typename std::iterator_traits<I>::value_type T;
  std::string whitespace = " \n\r\f\t";

  size_t pos = s.find_first_not_of(whitespace, 0);
  size_t next_pos = s.find_first_of(whitespace, pos+1);
  size_t i = 0;
  while (i < n && pos != s.npos) {
    if (next_pos != s.npos) {
      s[next_pos] = '\0';
    }
    *begin = NRLibPrivate::UnsafeParser<T>::ParseType(&s[pos]);
    ++begin;
    pos = s.find_first_not_of(whitespace, next_pos + 1);
    next_pos = s.find_first_of(whitespace, pos + 1);
    ++i;
  }

  if (i != n) {
    throw Exception("Not enough elements parsed from string.");
  }

  return begin;
}


#endif // NRLIB_STRINGTOOLS_HPP
