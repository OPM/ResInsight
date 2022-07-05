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

#ifndef OPM_IORDER_SET_HPP
#define OPM_IORDER_SET_HPP

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

namespace Opm {


/*
  Small class which implements a container which behaves roughly like
  std::set<T>, but the insert order is preserved - i.e. when iterating over the
  elements in the container they will come out in the order they have been
  inserted. If an element is added multiple times the order in the container
  will not be updated.

  The set has an erase() method which can be used to remove elements, otherwise
  the elements in the container are immutable.

  The elements are duplicated in the std::set<T> and std::vector<T>, and the
  class should not be used for large objects.
*/

template <typename T>
class IOrderSet {
public:
    using storage_type = typename std::vector<T>;
    using index_type = typename std::unordered_set<T>;
    using const_iter_type = typename storage_type::const_iterator;

private:
    index_type m_index;
    storage_type m_data;

public:
    IOrderSet() = default;
    IOrderSet(const index_type& index, const storage_type& data)
        : m_index(index)
        , m_data(data)
   {}

    std::size_t size() const {
        return this->m_index.size();
    }

    bool empty() const {
        return (this->size() == 0);
    }

    std::size_t count(const T& value) const {
        return this->m_index.count(value);
    }

    bool contains(const T& value) const {
        return (this->count(value) != 0);
    }

    bool insert(const T& value) {
        if (this->contains(value))
            return false;

        this->m_index.insert(value);
        this->m_data.push_back(value);
        return true;
    }

    std::size_t erase(const T& value) {
        if (!this->contains(value))
            return 0;

        this->m_index.erase(value);
        auto data_iter = std::find(this->m_data.begin(), this->m_data.end(), value);
        this->m_data.erase(data_iter);
        return 1;
    }

    const_iter_type begin() const {
        return this->m_data.begin();
    }

    const_iter_type end() const {
        return this->m_data.end();
    }

    const T& operator[](std::size_t i) const {
        return this->m_data.at(i);
    }

    const std::vector<T>& data() const {
        return this->m_data;
    };

    bool operator==(const IOrderSet<T>& data) const {
        return this->m_index == data.m_index &&
               this->data() == data.data();
    }

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(m_index);
        serializer(m_data);
    }
};
}

#endif
