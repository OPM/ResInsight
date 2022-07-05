/*
  Copyright 2021 Equinor ASA.

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

#ifndef ACTION_ENUMS_HPP
#define ACTION_ENUMS_HPP

#include <string>

namespace Opm {
namespace Action {

enum class Logical {
    AND,
    OR,
    END
};

enum class Comparator {
    EQUAL,
    GREATER,
    LESS,
    GREATER_EQUAL,
    LESS_EQUAL,
    INVALID
};


Comparator  comparator_from_int(int cmp_int);
std::string comparator_as_string(Comparator cmp);
Logical     logic_from_int(int int_logic);

}
}

#endif
