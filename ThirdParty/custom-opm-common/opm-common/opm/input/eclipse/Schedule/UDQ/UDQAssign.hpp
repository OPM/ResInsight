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
#include <unordered_set>
#include <vector>

#include <opm/input/eclipse/Schedule/UDQ/UDQSet.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQEnums.hpp>

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
        std::vector<std::string> input_selector;
        std::unordered_set<std::string> rst_selector;
        double value;
        std::size_t report_step;

        AssignRecord() = default;

        AssignRecord(const std::vector<std::string>& selector, double value_arg, std::size_t report_step_arg)
            : input_selector(selector)
            , value(value_arg)
            , report_step(report_step_arg)
        {}

        AssignRecord(const std::unordered_set<std::string>& selector, double value_arg, std::size_t report_step_arg)
            : rst_selector(selector)
            , value(value_arg)
            , report_step(report_step_arg)
        {}

        void eval(UDQSet& values) const {
            if (this->input_selector.empty() && this->rst_selector.empty())
                values.assign( this->value );
            else {
                if (this->rst_selector.empty())
                    values.assign(this->input_selector[0], this->value);
                else {
                    for (const auto& wgname : this->rst_selector)
                        values.assign(wgname, this->value);
                }
            }
        }

        bool operator==(const AssignRecord& data) const {
            return input_selector == data.input_selector &&
                   rst_selector == data.rst_selector &&
                   report_step == data.report_step &&
                   value == data.value;
        }

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(input_selector);
            serializer(rst_selector);
            serializer(value);
            serializer(report_step);
        }
    };

    UDQAssign();
    UDQAssign(const std::string& keyword, const std::vector<std::string>& selector, double value, std::size_t report_step);
    UDQAssign(const std::string& keyword, const std::unordered_set<std::string>& selector, double value, std::size_t report_step);

    static UDQAssign serializeObject();

    const std::string& keyword() const;
    UDQVarType var_type() const;
    void add_record(const std::vector<std::string>& selector, double value, std::size_t report_step);
    void add_record(const std::unordered_set<std::string>& rst_selector, double value, std::size_t report_step);
    UDQSet eval(const std::vector<std::string>& wells) const;
    UDQSet eval() const;
    std::size_t report_step() const;

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
