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

#include <fmt/format.h>
#include <opm/common/OpmLog/KeywordLocation.hpp>


namespace Opm {

std::string KeywordLocation::format(const std::string& msg_fmt) const {
    return fmt::format(msg_fmt,
                       fmt::arg("keyword", this->keyword),
                       fmt::arg("file", this->filename),
                       fmt::arg("line", this->lineno));
}

}
