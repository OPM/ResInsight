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

#include <fmt/format.h>
#include <stdexcept>

#include <opm/input/eclipse/Schedule/Action/Enums.hpp>

namespace Opm {
namespace Action {

Logical logic_from_int(int int_logic) {
    if (int_logic == 0)
        return Logical::END;

    if (int_logic == 1)
        return Logical::AND;

    if (int_logic == 2)
        return Logical::OR;

    throw std::logic_error("Unknown integer value");
}




Comparator comparator_from_int(int cmp_int) {
    switch (cmp_int) {
    case 1:
        return Comparator::GREATER;
    case 2:
        return Comparator::LESS;
    case 3:
        return Comparator::GREATER_EQUAL;
    case 4:
        return Comparator::LESS_EQUAL;
    case 5:
        return Comparator::EQUAL;
    default:
        throw std::logic_error(fmt::format("Integer value: {} could not be converted to ACTIONX comparator", cmp_int));
    }
}

std::string comparator_as_string(Comparator cmp) {
    if (cmp == Comparator::EQUAL)
        return "=";

    if (cmp == Comparator::GREATER)
        return ">";

    if (cmp == Comparator::LESS)
        return "<";

    if (cmp == Comparator::LESS_EQUAL)
        return "<=";

    if (cmp == Comparator::GREATER_EQUAL)
        return ">=";

    throw std::logic_error("Bug in opm/flow - should not be here");
}

}
}
