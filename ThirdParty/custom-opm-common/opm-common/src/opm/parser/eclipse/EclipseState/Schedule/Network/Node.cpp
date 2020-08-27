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
#include <stdexcept>

#include <opm/parser/eclipse/EclipseState/Schedule/Network/Node.hpp>

namespace Opm {
namespace Network {

Node::Node(const std::string& name) :
    m_name(name)
{}

const std::string& Node::name() const {
    return this->m_name;
}

const std::optional<std::string>& Node::target_group() const {
    return this->m_choke_target_group;
}

const std::optional<double>& Node::terminal_pressure() const {
    return this->m_terminal_pressure;
}

bool Node::add_gas_lift_gas() const {
    return this->m_add_gas_lift_gas;
}

bool Node::as_choke() const {
    return this->m_choke_target_group.has_value();
}

void Node::terminal_pressure(double pressure) {
    this->m_terminal_pressure = pressure;
}

void Node::add_gas_lift_gas(bool add_gas) {
    this->m_add_gas_lift_gas = add_gas;
}

void Node::as_choke(const std::string& target_group) {
    this->m_choke_target_group = target_group;
}


bool Node::operator==(const Node& other) const {
    return this->m_name == other.m_name &&
           this->m_terminal_pressure == other.m_terminal_pressure &&
           this->m_add_gas_lift_gas == other.m_add_gas_lift_gas &&
           this->m_choke_target_group == other.m_choke_target_group;
}


}
}
