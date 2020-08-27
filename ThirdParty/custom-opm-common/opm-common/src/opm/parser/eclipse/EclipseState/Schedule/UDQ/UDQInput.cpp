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


#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQDefine.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQAssign.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQInput.hpp>

namespace Opm {


UDQInput::UDQInput(const UDQIndex& index_arg, const UDQDefine& udq_define, const std::string& unit_arg) :
    index(index_arg),
    define(std::addressof(udq_define)),
    assign(nullptr),
    m_keyword(udq_define.keyword()),
    m_var_type(udq_define.var_type()),
    m_unit(unit_arg)
{}


UDQInput::UDQInput(const UDQIndex& index_arg, const UDQAssign& udq_assign, const std::string& unit_arg):
    index(index_arg),
    define(nullptr),
    assign(std::addressof(udq_assign)),
    m_keyword(udq_assign.keyword()),
    m_var_type(udq_assign.var_type()),
    m_unit(unit_arg)
{}

const std::string& UDQInput::unit() const {
    return this->m_unit;
}

const std::string& UDQInput::keyword() const {
    return this->m_keyword;
}

const UDQVarType& UDQInput::var_type() const {
    return this->m_var_type;
}

template<>
bool UDQInput::is<UDQAssign>() const {
    if (this->assign)
        return true;
    return false;
}


template<>
bool UDQInput::is<UDQDefine>() const {
    if (this->define)
        return true;
    return false;
}

template<>
const UDQAssign& UDQInput::get<UDQAssign>() const {
    if (this->assign)
        return *this->assign;

    throw std::runtime_error("Invalid get");
}


template<>
const UDQDefine& UDQInput::get<UDQDefine>() const {
    if (this->define)
        return *this->define;

    throw std::runtime_error("Invalid get");
}




}


