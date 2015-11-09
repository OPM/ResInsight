// $Id: tabularfile.cpp 1078 2012-09-25 11:13:53Z veralh $

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

#include "tabularfile.hpp"

#include <fstream>

#include "fileio.hpp"

#include "../exception/exception.hpp"

namespace NRLib {

TabularFile::TabularFile(const std::string& filename)
{
  size_t first_data_line, n_columns;
  bool read_last_line;
  std::string last_line;
  if (!CheckFile(filename, first_data_line, n_columns, read_last_line, last_line))
    throw FileFormatError("The format of " + filename + " is not supported.");

  ReadFromFile(filename, first_data_line, n_columns, read_last_line);
}


TabularFile::TabularFile(const std::string & filename,
                         size_t              first_data_line,
                         size_t              n_columns,
                         bool                read_last_line)
{
  ReadFromFile(filename, first_data_line, n_columns, read_last_line);
}


bool TabularFile::CheckFile(const std::string & filename,
                            size_t            & first_data_line,
                            size_t            & n_columns,
                            bool              & read_last_line,
                            std::string       & last_line)
{
  // Check if the last line of the file consists of data
  // and make initial guess if the last line shoud be read (not reading when equal to 0 or -999)
  std::ifstream in_file0;
  OpenRead(in_file0, filename);
  last_line = FindLastNonEmptyLine(in_file0);
  std::vector<std::string> tokens = GetTokens(last_line);
  read_last_line = true;
  if (!IsType<double>(tokens[0])) {
    read_last_line = false;
  }
  else {
    for (size_t i = 0; i < tokens.size(); ++i) {
      if (!IsType<double>(tokens[i]))
        read_last_line = false;
      else {
        if(atof(tokens[i].c_str()) == 0.0 || atof(tokens[i].c_str()) == -999.0)
          read_last_line = false;
      }
    }
  }
  std::ifstream in_file;
  OpenRead(in_file, filename);

  int line_number = 0;
  std::string line;
  while (GetNextNonEmptyLine(in_file, line_number, line)) {
    std::vector<std::string> tokens = GetTokens(line);
    if (IsType<double>(tokens[0])) {
      first_data_line = line_number;
      n_columns = tokens.size();
      for (size_t i = 0; i < n_columns; ++i) {
        if (!IsType<double>(tokens[i]))
          return false;
      }
      return true;
    }
  }
  return false;
}


void TabularFile::ReadFromFile(const std::string & filename,
                               size_t              first_data_line,
                               size_t              n_columns,
                               bool                read_last_line)
{
  std::ifstream in_file;
  OpenRead(in_file, filename);
  std::string line;

  columns_.resize(n_columns);
  std::vector<double> data(n_columns);

  // First line in file is line number 1. Read and discard up to first_data_line
  for (size_t i = 1; i < first_data_line; ++i) {
    std::getline(in_file, line);
  }
  std::string this_line, next_line;
  std::getline(in_file, this_line);
  while(std::getline(in_file, next_line)) {
    ParseAsciiArrayFast(this_line, data.begin(), n_columns);
    for (size_t i = 0; i < n_columns; ++i) {
      columns_[i].push_back(data[i]);
    }
    this_line = next_line;
  }
  // Reading last line
  if(read_last_line) {
    ParseAsciiArrayFast(this_line, data.begin(), n_columns);
    for (size_t i = 0; i < n_columns; ++i) {
      columns_[i].push_back(data[i]);
    }
  }
}


} // namespace NRLib
