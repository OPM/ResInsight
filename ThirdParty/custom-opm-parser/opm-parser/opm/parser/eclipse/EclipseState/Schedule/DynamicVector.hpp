/*
  Copyright 2013 Statoil ASA.

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


#ifndef DYNAMICVECTOR_HPP_
#define DYNAMICVECTOR_HPP_


#include <stdexcept>


#include <opm/parser/eclipse/EclipseState/Schedule/TimeMap.hpp>

namespace Opm {

    /*
      The DynamicVector<T> class is a thin wrapper around
      std::vector<T> with the following twists:

        - The vector is bound to a TimeMap instance.

        - The operator[] supports arbitrary assignment - the vector
          will grow as needed, however it will *not* grow beyond the
          length of the TimeMap.

        - The vector is created with a default value which will be
          used to 'fill in the holes' when growing; and that value
          will also be returned if you ask for values beyond the end
          of the vector.
    */

    template <class T>
    class DynamicVector {
    public:


        DynamicVector(const std::shared_ptr< const TimeMap > timeMap, T defaultValue) {
            m_timeMap = timeMap;
            m_defaultValue = defaultValue;
        }


        const T& operator[](size_t index) const {
            assertSize( index );

            if (index < m_data.size())
                return m_data[index];
            else
                return m_defaultValue;
        }


        const T& iget(size_t index) const {
            return (*this)[index];
        }



        T& operator[](size_t index) {
            assertSize( index );

            if (index >= m_data.size())
                m_data.resize( index + 1 , m_defaultValue);

            return m_data[index];
        }

        void iset(size_t index, T value) {
            (*this)[index] = value;
        }


    private:
        void assertSize(size_t index) const {
            if (index >= m_timeMap->size())
                throw std::range_error("Index value is out range.");
        }


        std::vector<T> m_data;
        std::shared_ptr< const TimeMap > m_timeMap;
        T m_defaultValue;
    };
}



#endif
