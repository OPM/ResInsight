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


#ifndef OPM_RECORD_VECTOR_HPP
#define OPM_RECORD_VECTOR_HPP

#include <vector>
#include <stdexcept>

/*
  A vector like container which will return the last valid element
  when the lookup index is out of range.
*/

namespace Opm {

template <typename T>
class RecordVector {
private:
    std::vector<T> m_data;
public:
    size_t size() const {
        return m_data.size();
    }

    T get(size_t index) const {
        if (m_data.size() > 0) {
            if (index >= m_data.size())
                return m_data.back();
            else
                return m_data[index];
        } else
            throw std::invalid_argument("Trying to get from empty RecordVector");
    }


    void push_back(T value) {
        m_data.push_back(value);
    }

    typename std::vector<T>::const_iterator begin() const {
        return m_data.begin();
    }


    typename std::vector<T>::const_iterator end() const {
        return m_data.end();
    }

};
}

#endif
