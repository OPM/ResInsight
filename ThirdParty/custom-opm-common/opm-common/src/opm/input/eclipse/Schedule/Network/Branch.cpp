/*
  Copyright 2020 Equinor ASA.

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


#include <opm/input/eclipse/Schedule/Network/Branch.hpp>

#include <stdexcept>

namespace Opm {
namespace Network {

namespace {

constexpr int invalid_vfp_table = 9999;

}


Branch Branch::serializeObject() {
    Branch object;
    return object;
}


Branch::Branch(const std::string& downtree_node, const std::string& uptree_node, int vfp_table, double alq) :
    m_downtree_node(downtree_node),
    m_uptree_node(uptree_node),
    m_vfp_table(vfp_table),
    m_alq_value(alq),
    m_alq_eq(AlqEQ::ALQ_INPUT)
{
}

Branch::Branch(const std::string& downtree_node, const std::string& uptree_node, int vfp_table, AlqEQ alq_eq):
    m_downtree_node(downtree_node),
    m_uptree_node(uptree_node),
    m_vfp_table(vfp_table),
    m_alq_eq(alq_eq)
{
    if (alq_eq == AlqEQ::ALQ_INPUT)
        throw std::logic_error("Wrong constructor - must supply ALQ value");
}

const std::string& Branch::uptree_node() const {
    return this->m_uptree_node;
}

const std::string& Branch::downtree_node() const {
    return this->m_downtree_node;
}

bool Branch::operator==(const Branch& other) const {
    return this->m_downtree_node == other.m_downtree_node &&
           this->m_uptree_node == other.m_uptree_node &&
           this->m_vfp_table == other.m_vfp_table &&
           this->m_alq_value == other.m_alq_value &&
           this->m_alq_eq == other.m_alq_eq;
}

Branch::AlqEQ Branch::AlqEqfromString(const std::string& input_string) {
    if (input_string == "NONE")
        return AlqEQ::ALQ_INPUT;

    if (input_string == "DENO")
        return AlqEQ::OIL_DENSITY;

    if (input_string == "DENG")
        return AlqEQ::GAS_DENSITY;

    throw std::invalid_argument("Invalid input for ALQ surface density eq: " + input_string);
}

std::optional<int> Branch::vfp_table() const {
    if (this->m_vfp_table == invalid_vfp_table)
        return {};
    else
        return this->m_vfp_table;
}

Branch::AlqEQ Branch::alq_eq() const {
    return this->m_alq_eq;
}

std::optional<double> Branch::alq_value() const {
    return this->m_alq_value;
}

}
}
