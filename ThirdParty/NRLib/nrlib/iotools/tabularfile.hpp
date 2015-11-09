// $Id: tabularfile.hpp 1078 2012-09-25 11:13:53Z veralh $

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

#ifndef NRLIB_IOTOOLS_TABULARFILE_HPP
#define NRLIB_IOTOOLS_TABULARFILE_HPP

#include <cassert>
#include <string>
#include <vector>

namespace NRLib {

/// Class for files with tabular data, i.e. data values separated by a delimiter.
/// Currently only supports doubles separated by spaces with an optional text header.
class TabularFile
{
public:
  explicit TabularFile(const std::string& filename);
  TabularFile(const std::string& filename, size_t first_data_line, size_t n_columns, bool read_last_line = true);

  /// Simple check of file. Just checks that the file contains tabular data.
  /// \param[out] first_data_line Number of lines with header data.
  static bool CheckFile(const std::string& filename,
                        size_t&            first_data_line,
                        size_t&            n_columns,
                        bool&              read_last_line,
                        std::string&       last_line);

  void ReadFromFile(const std::string& filename,
                    size_t first_data_line,
                    size_t n_columns,
                    bool read_last_line = true);

  size_t GetNColumns() const { return columns_.size(); }

  inline const std::vector<double>& GetColumn(size_t i) const;

private:
  std::vector<std::vector<double> > columns_;
};

// ========== INLINE FUNCTIONS =========

const std::vector<double>& TabularFile::GetColumn(size_t i) const
{
  assert(i < columns_.size());
  return columns_[i];
}

} // namespace NRLib

#endif // NRLIB_IOTOOLS_TABULARFILE_HPP
