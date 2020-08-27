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


#ifndef NETWORK_NODE_HPP
#define NETWORK_NODE_HPP

#include <optional>
#include <string>

namespace Opm {
namespace Network {

class Node {
public:
    Node() = default;
    Node(const std::string& name);

    const std::string& name() const;
    const std::optional<double>& terminal_pressure() const;
    bool as_choke() const;
    bool add_gas_lift_gas() const;
    const std::optional<std::string>& target_group() const;

    void terminal_pressure(double pressure);
    void add_gas_lift_gas(bool add_gas);
    void as_choke(const std::string& target_group);

    static Node serializeObject();
    bool operator==(const Node& other) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(m_name);
        serializer(m_terminal_pressure);
        serializer(m_add_gas_lift_gas);
        serializer(m_choke_target_group);
    }
private:
    std::string m_name;
    std::optional<double> m_terminal_pressure;
    std::optional<std::string> m_choke_target_group;
    bool m_add_gas_lift_gas = false;
};
}
}
#endif
