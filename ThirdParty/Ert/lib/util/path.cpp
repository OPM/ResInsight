/*
  Copyright (C) 2012  Statoil ASA, Norway.

  The file 'path_stack.c' is part of ERT - Ensemble based Reservoir Tool.

  ERT is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  ERT is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.

  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
  for more details.
*/

#include <string>

#include <ert/util/util.h>

#include "detail/util/path.hpp"

namespace ecl {

  namespace util {

    namespace path {

      /*
        The bizarre dive down into c_str() is to avoid inadvertendtly
        introducing a symbol which is creates a ABI compatibility issue between
        the libstdc++ librararies. A bug in the devtoolset compiler?
      */

      std::string dirname(const std::string& fname) {
        size_t last_slash = fname.rfind(UTIL_PATH_SEP_CHAR);
        if (last_slash == std::string::npos)
          return "";

        if (last_slash == 0)
          return fname.substr(0,1);
        else
          return fname.substr(0, last_slash);
      }

      std::string basename(const std::string& fname) {
        size_t end_pos = fname.rfind('.');
        size_t offset = fname.rfind(UTIL_PATH_SEP_CHAR);
        if (offset == std::string::npos)
          offset = 0;
        else
          offset += 1;

        {
          const char * c_str = fname.c_str();
          if (end_pos == std::string::npos || end_pos < offset)
            return util_alloc_string_copy( &c_str[offset] );

          return util_alloc_substring_copy(c_str, offset, end_pos - offset);
        }
      }


      std::string extension(const std::string& fname)  {
        size_t end_pos = fname.rfind('.');
        size_t last_slash = fname.rfind(UTIL_PATH_SEP_CHAR);
        if (end_pos == std::string::npos)
          return "";

        if (last_slash == std::string::npos || end_pos > last_slash) {
          const char * c_str = fname.c_str();
          return util_alloc_substring_copy( c_str, end_pos + 1, fname.size() - end_pos - 1);
        }

        return "";
      }

    }
  }
}
