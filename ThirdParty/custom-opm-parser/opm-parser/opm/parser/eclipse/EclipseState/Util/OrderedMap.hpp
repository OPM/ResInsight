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


namespace Opm {

template <typename T>
class OrderedMap {
private:
    std::unordered_map<std::string , size_t> m_map;
    std::vector<T> m_vector;

public:
    bool hasKey(const std::string& key) const {
        auto iter = m_map.find(key);
        if (iter == m_map.end())
            return false;
        else
            return true;
    }


    void insert(std::string key, T value) {
        if (hasKey(key)) {
            auto iter = m_map.find( key );
            size_t index = iter->second;
            m_vector[index] = value;
        } else {
            size_t index = m_vector.size();
            m_vector.push_back( value );
            m_map.insert( std::pair<std::string, size_t>(key , index));
        }
    }


    T& get(const std::string& key) {
        auto iter = m_map.find( key );
        if (iter == m_map.end())
            throw std::invalid_argument("Key not found:" + key);
        else {
            size_t index = iter->second;
            return get(index);
        }
    }


    T& get(size_t index) {
        if (index >= m_vector.size())
            throw std::invalid_argument("Invalid index");
        return m_vector[index];
    }

    const T& get(const std::string& key) const {
        auto iter = m_map.find( key );
        if (iter == m_map.end())
            throw std::invalid_argument("Key not found:" + key);
        else {
            size_t index = iter->second;
            return get(index);
        }
    }


    const T& get(size_t index) const {
        if (index >= m_vector.size())
            throw std::invalid_argument("Invalid index");
        return m_vector[index];
    }


    T* getPtr(const std::string& key) const {
        auto iter = m_map.find( key );
        if (iter == m_map.end())
            throw std::invalid_argument("Key not found:" + key);
        else {
            size_t index = iter->second;
            return getPtr(index);
        }
    }

    T* getPtr(size_t index) const {
        if (index >= m_vector.size())
            throw std::invalid_argument("Invalid index");
        return &m_vector[index];
    }


    size_t size() const {
        return m_vector.size();
    }


    typename std::vector<T>::const_iterator begin() const {
        return m_vector.begin();
    }


    typename std::vector<T>::const_iterator end() const {
        return m_vector.end();
    }
};
}

#endif
