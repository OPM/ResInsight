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
#ifndef WELL_ORDER_HPP
#define WELL_ORDER_HPP

#include <initializer_list>
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>

namespace Opm {

/*
  The purpose of this small class is to ensure that well and group name always
  come in the order they are defined in the deck.
*/


using Map = std::unordered_map<std::string, std::size_t>;

class NameOrder {
public:
    NameOrder() = default;
    explicit NameOrder(std::initializer_list<std::string> names);
    explicit NameOrder(const std::vector<std::string>& names);
    void add(const std::string& name);
    std::vector<std::string> sort(std::vector<std::string> names) const;
    const std::vector<std::string>& names() const;
    bool has(const std::string& wname) const;
    std::size_t size() const;

    template<class Serializer>
    void serializeOp(Serializer& serializer) {
        serializer.template map<Map, false>(m_index_map);
        serializer(m_name_list);
    }

    static NameOrder serializeObject();

    const std::string& operator[](std::size_t index) const;
    bool operator==(const NameOrder& other) const;
    std::vector<std::string>::const_iterator begin() const;
    std::vector<std::string>::const_iterator end() const;

private:
    Map m_index_map;
    std::vector<std::string> m_name_list;
};


class GroupOrder {
public:
    GroupOrder() = default;
    explicit GroupOrder(std::size_t max_groups);
    void add(const std::string& name);
    const std::vector<std::string>& names() const;
    bool has(const std::string& wname) const;
    std::vector<std::optional<std::string>> restart_groups() const;

    template<class Serializer>
    void serializeOp(Serializer& serializer) {
        serializer(m_name_list);
        serializer(m_max_groups);
    }
    static GroupOrder serializeObject();

    bool operator==(const GroupOrder& other) const;
    std::vector<std::string>::const_iterator begin() const;
    std::vector<std::string>::const_iterator end() const;

private:
    std::vector<std::string> m_name_list;
    std::size_t m_max_groups;

};




}
#endif
