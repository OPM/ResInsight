/*
  Copyright 2021 Equinor ASA.

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
#include <algorithm>
#include <opm/input/eclipse/Schedule/Well/NameOrder.hpp>

namespace Opm {

void NameOrder::add(const std::string& name) {
    auto iter = this->m_index_map.find( name );
    if (iter == this->m_index_map.end()) {
        std::size_t insert_index = this->m_name_list.size();
        this->m_index_map.emplace( name, insert_index );
        this->m_name_list.push_back( name );
    }
}

NameOrder::NameOrder(const std::vector<std::string>& names) {
    for (const auto& w : names)
        this->add(w);
}

NameOrder::NameOrder(std::initializer_list<std::string> names) {
    for (const auto& w : names)
        this->add(w);
}

bool NameOrder::has(const std::string& wname) const {
    return (this->m_index_map.count(wname) != 0);
}


const std::vector<std::string>& NameOrder::names() const {
    return this->m_name_list;
}

std::vector<std::string> NameOrder::sort(std::vector<std::string> names) const {
    std::sort(names.begin(), names.end(), [this](const std::string& w1, const std::string& w2) -> bool { return this->m_index_map.at(w1) < this->m_index_map.at(w2); });
    return names;
}

NameOrder NameOrder::serializeObject() {
    NameOrder wo;
    wo.add("W1");
    wo.add("W2");
    wo.add("W3");
    return wo;
}

std::vector<std::string>::const_iterator NameOrder::begin() const {
    return this->m_name_list.begin();
}

std::vector<std::string>::const_iterator NameOrder::end() const {
    return this->m_name_list.end();
}

bool NameOrder::operator==(const NameOrder& other) const {
    return this->m_index_map == other.m_index_map &&
           this->m_name_list == other.m_name_list;
}

std::size_t NameOrder::size() const {
    return this->m_name_list.size();
}

const std::string& NameOrder::operator[](std::size_t index) const {
    return this->m_name_list.at(index);
}


/********************************************************************************/

GroupOrder::GroupOrder(std::size_t max_groups)
{
    this->m_max_groups = max_groups;
    this->add("FIELD");
}



void GroupOrder::add(const std::string& gname) {
    auto iter = std::find(this->m_name_list.begin(), this->m_name_list.end(), gname);
    if (iter == this->m_name_list.end())
        this->m_name_list.push_back( gname );
}


bool GroupOrder::has(const std::string& gname) const {
    auto iter = std::find(this->m_name_list.begin(), this->m_name_list.end(), gname);
    return (iter != this->m_name_list.end());
}


const std::vector<std::string>& GroupOrder::names() const {
    return this->m_name_list;
}


GroupOrder GroupOrder::serializeObject() {
    GroupOrder go(123);
    go.add("G1");
    go.add("G2");
    return go;
}

std::vector<std::optional<std::string>> GroupOrder::restart_groups() const {
    const auto& input_groups = this->names();
    std::vector<std::optional<std::string>> groups{ this->m_max_groups + 1 };
    for (std::size_t index = 1; index < input_groups.size(); index++)
        groups[index - 1] = input_groups[index];
    groups.back() = input_groups[0];
    return groups;
}

std::vector<std::string>::const_iterator GroupOrder::begin() const {
    return this->m_name_list.begin();
}

std::vector<std::string>::const_iterator GroupOrder::end() const {
    return this->m_name_list.end();
}

bool GroupOrder::operator==(const GroupOrder& other) const {
    return this->m_max_groups == other.m_max_groups &&
           this->m_name_list == other.m_name_list;
}
}
