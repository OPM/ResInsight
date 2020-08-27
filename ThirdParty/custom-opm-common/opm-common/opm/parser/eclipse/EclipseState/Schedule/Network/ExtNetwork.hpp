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


#ifndef EXT_NETWORK_HPP
#define EXT_NETWORK_HPP

#include <map>
#include <optional>
#include <string>
#include <vector>

#include <opm/parser/eclipse/EclipseState/Schedule/Network/Branch.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Network/Node.hpp>

namespace Opm {
namespace Network {


class ExtNetwork {
public:
    ExtNetwork() = default;
    bool active() const;
    bool has_node(const std::string& name) const;
    void add_node(Node node);
    void add_branch(Branch branch);
    void drop_branch(const std::string& uptree_node, const std::string& downtree_node);
    const Node& node(const std::string& name) const;
    const Node& root() const;
    std::vector<Branch> downtree_branches(const std::string& node) const;
    std::optional<Branch> uptree_branch(const std::string& node) const;


    bool operator==(const ExtNetwork& other) const;
    static ExtNetwork serializeObject();

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer.vector(m_branches);
        serializer.map(m_nodes);
    }

private:
    std::vector<Branch> m_branches;
    std::map<std::string, Node> m_nodes;
};

}
}
#endif
