/*
  Copyright 2019 Equinor ASA.

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

#ifndef ACTIONX_CONDITION_HPP
#define ACTIONX_CONDITION_HPP

#include <string>
#include <vector>

#include <opm/common/OpmLog/Location.hpp>

namespace Opm {

namespace Action {


class Quantity {
public:
    Quantity() = default;

    Quantity(const std::string& arg) :
        quantity(arg)
    {}

    void add_arg(const std::string& arg);
    std::string quantity;
    std::vector<std::string> args;

    bool operator==(const Quantity& data) const {
        return quantity == data.quantity &&
               args == data.args;
    }

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(quantity);
        serializer(args);
    }
};



class Condition {
public:

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


    Condition() = default;
    Condition(const std::vector<std::string>& tokens, const Location& location);


    Quantity lhs;
    Quantity rhs;
    Logical logic = Logical::END;
    Comparator cmp = Comparator::INVALID;
    std::string cmp_string;

    bool operator==(const Condition& data) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        lhs.serializeOp(serializer);
        rhs.serializeOp(serializer);
        serializer(logic);
        serializer(cmp);
        serializer(cmp_string);
    }
};


}
}

#endif
