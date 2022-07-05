/*
  Copyright 2014 Statoil ASA.

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

#ifndef OPM_ORDERED_MAP_HPP
#define OPM_ORDERED_MAP_HPP

#include <unordered_map>
#include <vector>
#include <string>
#include <stdexcept>
#include <iterator>
#include <sstream>
#include <set>
#include <cctype>
#include <algorithm>

namespace Opm {

namespace OrderedMapDetail
{

template<class T, class A>
std::string
findSimilarStrings(std::string str,
                   const std::vector<std::pair<std::string, T>,A>& storage)
{
    auto toUpper = [](const char c){ return std::toupper(c);};
    std::transform(str.begin(), str.end(), str.begin(), toUpper);
    std::set<std::string> alternatives;

    for(const auto& entry: storage)
    {
        std::string upper = entry.first;
        std::transform(upper.begin(), upper.end(), upper.begin(),
                       toUpper);

        if(upper.find(str) != std::string::npos || str.find(upper) != std::string::npos)
        {
            alternatives.insert(entry.first);
        }
    }

    if (alternatives.empty())
    {
        return {};
    }

    std::stringstream concated;
    std::copy(alternatives.begin(), alternatives.end(),
              std::ostream_iterator<std::string>(concated, ", "));
    auto concatedStr = concated.str();
    return concatedStr.substr(0, concatedStr.size()-2);
}

template<std::size_t MAX_CHARS>
class TruncatedStringHash
{
public:
    std::size_t operator()(const std::string_view& key) const
    {
        return hasher(key.substr(0, MAX_CHARS));
    }
private:
    std::hash<std::string_view> hasher;
};


template<>
class TruncatedStringHash<std::string::npos> : public std::hash<std::string_view>
{};

template<std::size_t MAX_CHARS>
struct TruncatedStringEquals
{
    bool operator()(const std::string& str1, const std::string& str2) const
    {
        return str1.substr(0, MAX_CHARS) == str2.substr(0, MAX_CHARS);
    }
};

template<>
struct TruncatedStringEquals<std::string::npos> : public std::equal_to<std::string>
{};

} // end namespace detail

/// \brief A map with iteration in the order of insertion.
///
/// Each entry has an associated index indicating when a value with that key was
/// first inserted. When itering over it's entries values with lower insertion index
/// are traversed before ones with an higher insertion index.
///
/// \tparam MAX_CHARS The maximum number of characters that are use a keys. Default is
///                   std::string::npos, which honors all characters. Any keys with the
///                   same first MAX_CHARS characters are considered equal.
///
template <typename T, std::size_t MAX_CHARS = std::string::npos>
class OrderedMap {
public:
    using storage_type = typename std::vector<std::pair<std::string,T>>;
    using index_type = typename std::unordered_map<std::string,std::size_t,
                                                   Opm::OrderedMapDetail::TruncatedStringHash<MAX_CHARS>,
                                                   Opm::OrderedMapDetail::TruncatedStringEquals<MAX_CHARS>>;
    using iter_type = typename storage_type::iterator;
    using const_iter_type = typename storage_type::const_iterator;

private:
    index_type m_map;
    storage_type m_vector;

public:

    OrderedMap() = default;

    OrderedMap(const index_type& index, const storage_type& storage)
        : m_map(index)
        , m_vector(storage)
    {
    }

    const index_type& getIndex() const { return m_map; }

    const storage_type& getStorage() const { return m_vector; }

    std::size_t count(const std::string& key) const {
        return this->m_map.count(key);
    }



    T& operator[](const std::string& key) {
        if (this->count(key) == 0)
            this->insert( std::make_pair(key, T()));

        return this->at(key);
    }


    std::size_t erase(const std::string& key) {
        if (this->count(key) == 0)
            return 0;

        std::size_t index = this->m_map.at(key);
        this->m_map.erase(key);
        this->m_vector.erase(this->m_vector.begin() + index);

        for (const auto& index_pair : this->m_map) {
            auto target_index = index_pair.second;
            if (target_index > index)
                target_index--;

            this->m_map[index_pair.first] = target_index;
        }
        return 1;
    }


    void insert(std::pair<std::string,T> key_value_pair) {
        if (this->count(key_value_pair.first) > 0) {
            auto iter = m_map.find( key_value_pair.first );
            size_t index = iter->second;
            m_vector[index] = key_value_pair;
        } else {
            size_t index = m_vector.size();
            this->m_map.emplace(key_value_pair.first, index);
            this->m_vector.push_back( std::move( key_value_pair ) );
        }
    }


    T& get(const std::string& key) {
        auto iter = m_map.find( key );
        if (iter == m_map.end())
        {
            using namespace std::string_literals;
            auto startsWithSame = OrderedMapDetail::findSimilarStrings(key, m_vector);
            if (!startsWithSame.empty())
            {
                startsWithSame = " Similar entries are "s +
                    startsWithSame + "."s;
            }
            throw std::invalid_argument("Key "s + key + " not found."s
                                        + startsWithSame);
        }
        else {
            size_t index = iter->second;
            return iget(index);
        }
    }


    T& iget(size_t index) {
        if (index >= m_vector.size())
            throw std::invalid_argument("Invalid index");
        return m_vector[index].second;
    }

    const T& get(const std::string& key) const {
        const auto& iter = this->m_map.find( key );
        if (iter == m_map.end())
        {
            auto startsWithSame = OrderedMapDetail::findSimilarStrings(key, m_vector);
            if (!startsWithSame.empty())
            {
                startsWithSame = std::string(" Similar entries are ") +
                    startsWithSame + std::string(".");
            }
            using namespace std::string_literals;
            throw std::invalid_argument("Key "s + key + " not found."s
                                        + startsWithSame);
        }
        else {
            size_t index = iter->second;
            return iget(index);
        }
    }


    const T& iget(size_t index) const {
        if (index >= m_vector.size())
        {
            using namespace std::string_literals;
            throw std::invalid_argument("Invalid index "s +
                                        std::to_string(index) +
                                        " is larger than container size"s);
        }
        return m_vector[index].second;
    }

    const T& at(size_t index) const {
        return this->iget(index);
    }

    const T& at(const std::string& key) const {
        return this->get(key);
    }

    T& at(size_t index) {
        return this->iget(index);
    }

    T& at(const std::string& key) {
        return this->get(key);
    }

    size_t size() const {
        return m_vector.size();
    }


    const_iter_type begin() const {
        return m_vector.begin();
    }


    const_iter_type end() const {
        return m_vector.end();
    }

    iter_type begin() {
        return m_vector.begin();
    }

    iter_type end() {
        return m_vector.end();
    }

    iter_type find(const std::string& key) {
        const auto map_iter = this->m_map.find(key);
        if (map_iter == this->m_map.end())
            return this->m_vector.end();

        return std::next(this->m_vector.begin(), map_iter->second);
    }

    const_iter_type find(const std::string& key) const {
        const auto map_iter = this->m_map.find(key);
        if (map_iter == this->m_map.end())
            return this->m_vector.end();

        return std::next(this->m_vector.begin(), map_iter->second);
    }

    template<size_t n>
    bool operator==(const OrderedMap<T,n>& data) const {
        return this->getIndex() == data.getIndex() &&
               this->getStorage() == data.getStorage();
    }

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(m_map);
        serializer.vector(m_vector);
    }
};
}

#endif
