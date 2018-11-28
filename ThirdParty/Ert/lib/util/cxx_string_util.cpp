/*
  Copyright (C) 2018  Equinor Statoil ASA, Norway.

  The file 'cxx_string_util.cpp' is part of ERT - Ensemble based Reservoir Tool.

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
#include <stdarg.h>


namespace ecl {
  namespace util {

    std::string string_format(const char* fmt, ...) {
      int length;
      std::string s;
      {
        va_list va;
        va_start(va, fmt);
        length = vsnprintf(NULL, 0, fmt, va);
        va_end(va);
      }
      s.resize(length + 1);
      {
        va_list va;
        va_start(va, fmt);
        vsprintf((char *) s.data(), fmt, va);
        va_end(va);
      }
      return s;
    }
  }
}
