// $Id: stringtools.cpp 1068 2012-09-18 11:21:53Z perroe $

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

#include "stringtools.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <sstream>

#define BOOST_FILESYSTEM_VERSION 2
#include <boost/filesystem.hpp>

using namespace NRLib;

template <>
std::string NRLib::ParseType<std::string>(const std::string& s)
{
  return s; //Just to ensure that whitespace-containing strings survive.
}


std::string
NRLib::GetPath(const std::string& filename)
{
  boost::filesystem::path path(filename);
  return path.parent_path().file_string();
}


std::string
NRLib::GetExtension(const std::string& filename)
{
  boost::filesystem::path path(filename);
  return path.extension();
}


std::string
NRLib::RemovePath(const std::string& filename)
{
  boost::filesystem::path path(filename);
  return path.filename();
}


std::string
NRLib::PrependDir(const std::string& prefix,
                   const std::string& str)
{
  boost::filesystem::path path(prefix);
  path /= str;
  return path.file_string();
}


std::string
NRLib::ReplaceExtension(const std::string& filename,
                         const std::string& extension)
{
  boost::filesystem::path file(filename);
  file.replace_extension(extension);
  return file.file_string();
}

std::string
NRLib::AddExtension(const std::string& filename,
                    const std::string& extension)
{
  std::string new_filename = filename + "." + extension;
  return new_filename;
}

std::string
NRLib::GetStem(const std::string& filename)
{
  boost::filesystem::path path(filename);
  return path.stem();
}


std::vector<std::string>
NRLib::GetTokens(const std::string& s)
{
  std::vector<std::string> v;
  std::istringstream iss(s);
  std::string tmp;
  while(iss >> tmp) {
    v.push_back(tmp);
  }
  return v;
}

std::vector<std::string>
NRLib::GetQuotedTokens(const std::string& s)
{
  std::vector<std::string> v;
  std::istringstream iss(s);
  std::string tmp;
  std::string char_tmp;
  size_t quotation_last_position = 0;
  while(iss >> tmp) {
    char_tmp = tmp.substr(0, 1);
    if (char_tmp != "\"")
      v.push_back(tmp);
    else{
      quotation_last_position = s.find_first_of("\"", quotation_last_position + 1);
      size_t first_quotation_mark = quotation_last_position;
      quotation_last_position = s.find_first_of("\"", quotation_last_position + 1);
      size_t second_quotation_mark = quotation_last_position;
      std::string quotation = s.substr(first_quotation_mark + 1, (second_quotation_mark - first_quotation_mark - 1));
      v.push_back(quotation);
      if (char_tmp == tmp)
        iss >> tmp;
      while (tmp.substr((tmp.size() - 1), 1) != "\"")
        iss >> tmp;
    }
  }
  return v;
}

void
NRLib::Substitute(std::string       & text,
                   const std::string & out,
                   const std::string & in)
{
  std::string::size_type len = in.size();
  std::string::size_type pos = text.find(out);
  while (pos != std::string::npos) {
    text.replace(pos, len, in);
    pos = text.find(out);
  }
}


std::string
NRLib::Uppercase(const std::string& text)
{
  std::string out = text;
  for (size_t i = 0; i < out.length(); ++i)
    out[i] = static_cast<char>(std::toupper(out[i]));
  return out;
}

bool
NRLib::IsNumber(const std::string & s)
{
  std::istringstream inStream(s);
  double inValue = 0.0;
  if (inStream >> inValue)
    return true;
  else
    return false;
}

std::string
NRLib::Chomp(const std::string& s)
{
  std::string out = s;

  size_t first, last;
  first = out.find_first_not_of(" ");
  last = out.find_last_not_of(" ");

  out.erase(last + 1);
  out.erase(0, first);

  return out;
}


