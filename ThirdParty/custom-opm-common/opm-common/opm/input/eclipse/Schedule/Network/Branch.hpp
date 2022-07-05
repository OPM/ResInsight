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


#ifndef NETWORK_BRANCH_HPP
#define NETWORK_BRANCH_HPP

#include <optional>
#include <string>

namespace Opm {
namespace Network {

class Branch {
public:

    enum class AlqEQ {
        OIL_DENSITY,
        GAS_DENSITY,
        ALQ_INPUT
    };


    static AlqEQ AlqEqfromString(const std::string& input_string);

    Branch() = default;
    Branch(const std::string& downtree_node, const std::string& uptree_node, int vfp_table, double alq);
    Branch(const std::string& downtree_node, const std::string& uptree_node, int vfp_table, AlqEQ alq_eq);

    const std::string& downtree_node() const;
    const std::string& uptree_node() const;
    std::optional<int> vfp_table() const;
    AlqEQ alq_eq() const;
    std::optional<double> alq_value() const;

    static Branch serializeObject();
    bool operator==(const Branch& other) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(m_downtree_node);
        serializer(m_uptree_node);
        serializer(m_vfp_table);
        serializer(m_alq_value);
        serializer(m_alq_eq);
    }
private:
    std::string m_downtree_node;
    std::string m_uptree_node;
    int m_vfp_table;
    std::optional<double> m_alq_value;
    AlqEQ m_alq_eq;
};

}
}
#endif
