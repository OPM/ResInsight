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

#include <opm/parser/eclipse/EclipseState/Schedule/Group/GTNode.hpp>

namespace Opm {

GTNode::GTNode(const Group& group_arg, std::size_t level, const std::optional<std::string>& parent_name) :
    m_group(group_arg),
    m_level(level),
    m_parent_name(parent_name)
{
}

const std::string& GTNode::name() const {
    return this->m_group.name();
}

const Group& GTNode::group() const {
    return this->m_group;
}

const std::string& GTNode::parent_name() const {
    if (this->m_parent_name.has_value())
        return *this->m_parent_name;

    throw std::invalid_argument("Tried to access parent of root in GroupTree. Root: " + this->name());
}


void GTNode::add_well(const Well& well) {
    this->m_wells.push_back(well);
}

void GTNode::add_group(const GTNode& child_group) {
    this->m_child_groups.push_back(child_group);
}

const std::vector<Well>& GTNode::wells() const {
    return this->m_wells;
}

const std::vector<GTNode>& GTNode::groups() const {
    return this->m_child_groups;
}

std::vector<const GTNode*> GTNode::all_nodes() const {
    std::vector<const GTNode*> nodes { this };

    for (const auto& child_group : m_child_groups) {
        const auto child_nodes { child_group.all_nodes() } ;
        nodes.insert(nodes.end(), child_nodes.begin(), child_nodes.end());
    }

    return nodes;
}

std::size_t GTNode::level() const {
    return this->m_level;
}

}
