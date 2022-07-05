/*
  Copyright 2020 Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef INFO_LOGGER_HPP
#define INFO_LOGGER_HPP

#include <cstddef>
#include <string>

namespace Opm {

/*
  A small utility class to use in the situation where the first N messages
  should go to OpmLog::info() and then the subsequent messages should go to
  OpmLog::note()
*/

class InfoLogger {
public:
    InfoLogger(const std::string& context_arg, std::size_t info_limit);
    void operator()(const std::string& msg);
private:
    std::string context;
    std::size_t info_limit;
    std::size_t info_count=0;
    void (*log_function)(const std::string&);
};

}

#endif
