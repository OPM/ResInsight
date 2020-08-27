/*
  Copyright 2018 Statoil ASA.

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


#ifndef UDQASSIGN_HPP_
#define UDQASSIGN_HPP_

#include <string>
#include <vector>

#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQSet.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQEnums.hpp>

namespace Opm {


class UDQAssign{
public:

    /*
      If the same keyword is assigned several times the different assignment
      records are assembled in one UDQAssign instance. This is an attempt to
      support restart in a situation where a full UDQ ASSIGN statement can be
      swapped with a UDQ DEFINE statement.
    */
    struct AssignRecord {
        std::vector<std::string> selector;
        double value;

        bool operator==(const AssignRecord& data) const {
            return selector == data.selector &&
                   value == data.value;
        }

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(selector);
            serializer(value);
        }
    };

    UDQAssign();
    UDQAssign(const std::string& keyword, const std::vector<std::string>& selector, double value);

    static UDQAssign serializeObject();

    const std::string& keyword() const;
    UDQVarType var_type() const;
    void add_record(const std::vector<std::string>& selector, double value);
    UDQSet eval(const std::vector<std::string>& wells) const;

    bool operator==(const UDQAssign& data) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(m_keyword);
        serializer(m_var_type);
        serializer.vector(records);
    }

private:
    std::string m_keyword;
    UDQVarType m_var_type;
    std::vector<AssignRecord> records;
};
}



#endif
