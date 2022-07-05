/*
  Copyright 2021 Equinor ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
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

#include <opm/io/eclipse/rst/udq.hpp>
#include <opm/output/eclipse/UDQDims.hpp>

namespace Opm {
namespace RestartIO {

RstUDQ::RstDefine::RstDefine(const std::string& expression_arg, UDQUpdate status_arg) :
    expression(expression_arg),
    status(status_arg)
{}


void RstUDQ::RstAssign::update_value(const std::string& name_arg, double new_value) {
    auto current_value = this->value.value_or(new_value);
    if (current_value != new_value)
        throw std::logic_error(fmt::format("Internal error: the UDQ {} changes value {} -> {} during restart load", name_arg, current_value, new_value));

    this->value = new_value;
}


RstUDQ::RstUDQ(const std::string& name_arg, const std::string& unit_arg, const std::string& define_arg, UDQUpdate update_arg)
    : name(name_arg)
    , unit(unit_arg)
    , var_type(UDQ::varType(name_arg))
    , data(RstDefine{define_arg,update_arg})
{
}

RstUDQ::RstUDQ(const std::string& name_arg, const std::string& unit_arg)
    : name(name_arg)
    , unit(unit_arg)
    , var_type(UDQ::varType(name_arg))
    , data(RstAssign{})
{
}



bool RstUDQ::is_define() const {
    return std::holds_alternative<RstDefine>(this->data);
}

void RstUDQ::add_value(const std::string& wgname, double value) {
    if (this->is_define()) {
        auto& def = std::get<RstDefine>(this->data);
        def.values.emplace_back(wgname, value);
    } else {
        auto& assign = std::get<RstAssign>(this->data);
        assign.update_value(this->name, value);
        assign.selector.insert(wgname);
    }
}

void RstUDQ::add_value(double value) {
    if (this->is_define()) {
        auto& def = std::get<RstDefine>(this->data);
        def.field_value = value;
    } else {
        auto& assign = std::get<RstAssign>(this->data);
        assign.update_value(this->name, value);
    }
}

double RstUDQ::assign_value() const {
    const auto& assign = std::get<RstAssign>(this->data);
    return assign.value.value();
}

const std::unordered_set<std::string>& RstUDQ::assign_selector() const {
    const auto& assign = std::get<RstAssign>(this->data);
    return assign.selector;
}

const std::string& RstUDQ::expression() const {
    const auto& define = std::get<RstDefine>(this->data);
    return define.expression;
}

const std::vector<std::pair<std::string, double>>& RstUDQ::values() const {
    const auto& define = std::get<RstDefine>(this->data);
    return define.values;
}

std::optional<double> RstUDQ::field_value() const {
    const auto& define = std::get<RstDefine>(this->data);
    return define.field_value;
}

RstUDQActive::RstRecord::RstRecord(UDAControl c, std::size_t i, std::size_t u1, std::size_t u2)
    : control(c)
    , input_index(i)
    , use_count(u1)
    , wg_offset(u2)
{}

RstUDQActive::RstUDQActive(const std::vector<int>& iuad_arg, const std::vector<int>& iuap, const std::vector<int>& igph)
{
    auto uda_size = UDQDims::entriesPerIUAD();
    for (std::size_t iuad_index = 0; iuad_index < iuad_arg.size() / uda_size; iuad_index++) {
        auto offset = iuad_index * uda_size;
        this->iuad.emplace_back( UDQ::udaControl(iuad_arg[offset + 0]),
                                 iuad_arg[offset + 1] - 1,
                                 iuad_arg[offset + 3],
                                 iuad_arg[offset + 4] - 1);
    }

    std::transform(iuap.begin(), iuap.end(), std::back_inserter(this->wg_index), [](const int& value) { return value - 1;});

    for (const auto& int_phase : igph) {
        Phase phase{Phase::OIL};

        if (int_phase == 1)
            phase = Phase::OIL;

        if (int_phase == 2)
            phase = Phase::WATER;

        if (int_phase == 3)
            phase = Phase::GAS;

        this->ig_phase.push_back(phase);
    };
}

}
}
