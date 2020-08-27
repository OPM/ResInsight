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

namespace Opm {

template <typename K, typename T>
class OrderedMap {
public:
    using storage_type = typename std::vector<std::pair<K,T>>;
    using index_type = typename std::unordered_map<K,std::size_t>;
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

    std::size_t count(const K& key) const {
        return this->m_map.count(key);
    }



    T& operator[](const K& key) {
        if (this->count(key) == 0)
            this->insert( std::make_pair(key, T()));

        return this->at(key);
    }


    std::size_t erase(const K& key) {
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


    void insert(std::pair<K,T> key_value_pair) {
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


    T& get(const K& key) {
        auto iter = m_map.find( key );
        if (iter == m_map.end())
            throw std::invalid_argument("Key not found:");
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

    const T& get(const K& key) const {
        const auto& iter = this->m_map.find( key );
        if (iter == m_map.end())
            throw std::invalid_argument("Key not found: ??");
        else {
            size_t index = iter->second;
            return iget(index);
        }
    }


    const T& iget(size_t index) const {
        if (index >= m_vector.size())
            throw std::invalid_argument("Invalid index");
        return m_vector[index].second;
    }

    const T& at(size_t index) const {
        return this->iget(index);
    }

    const T& at(const K& key) const {
        return this->get(key);
    }

    T& at(size_t index) {
        return this->iget(index);
    }

    T& at(const K& key) {
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

    iter_type find(const K& key) {
        const auto map_iter = this->m_map.find(key);
        if (map_iter == this->m_map.end())
            return this->m_vector.end();

        return std::next(this->m_vector.begin(), map_iter->second);
    }

    const_iter_type find(const K& key) const {
        const auto map_iter = this->m_map.find(key);
        if (map_iter == this->m_map.end())
            return this->m_vector.end();

        return std::next(this->m_vector.begin(), map_iter->second);
    }

    bool operator==(const OrderedMap<K,T>& data) const {
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
