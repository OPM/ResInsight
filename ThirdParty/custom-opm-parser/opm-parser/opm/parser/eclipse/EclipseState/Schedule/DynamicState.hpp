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


#ifndef DYNAMICSTATE_HPP_
#define DYNAMICSTATE_HPP_

#include <stdexcept>

#include <opm/parser/eclipse/EclipseState/Schedule/TimeMap.hpp>

#include <ert/util/ssize_t.h>


namespace Opm {

    /**
       The DynamicState<T> class is designed to hold information about
       properties with the following semantics:

         1. The property can be updated repeatedly at different
            timesteps; observe that the class does not support
            operator[] - only updates with weakly increasing timesteps
            are supported.

         2. At any point in the time the previous last set value
            applies.

       The class is very much tailored to support the Schedule file of
       Eclipse where a control applied at time T will apply
       indefinitely, or until explicitly set to a different value.

       The update() method returns true if the updated value is
       different from the current value, this implies that the
       class<T> must support operator!=
    */



    template <class T>
    class DynamicState {
    public:


        DynamicState(const std::shared_ptr< const TimeMap > timeMap, T initialValue) {
            m_timeMap = timeMap;
            init( initialValue );
        }

        void globalReset( T newValue ) {
            init( newValue );
        }


        const T& back() const {
            if (m_data.size() > 0)
                return m_data.back();
            else
                return m_initialValue;
        }

        const T& at(size_t index) const {
            if (index >= m_timeMap->size())
                throw std::range_error("Index value is out range.");

            if (index >= m_data.size())
                return m_currentValue;

            return m_data.at(index);
        }


        const T& operator[](size_t index) const {
            return at(index);
        }


        const T& get(size_t index) const {
            return this->at( index );
        }



        void updateInitial(T initialValue) {
            if (m_initialValue != initialValue) {
                size_t index;
                m_initialValue = initialValue;
                if (m_initialRange > 0) {
                    for (index = 0; index < m_initialRange; index++)
                        m_data[index] = m_initialValue;
                } else
                    m_currentValue = initialValue;
            }
        }


        size_t size() const {
            return m_data.size();
        }


        /**
           If the current value has been changed the method will
           return true, otherwise it will return false.
        */
        bool update(size_t index , T value) {
            bool change = (value != m_currentValue);
            if (index >= (m_timeMap->size()))
                throw std::range_error("Index value is out range.");

            if (m_data.size() > 0) {
                if (index < (m_data.size() - 1))
                    throw std::invalid_argument("Elements must be added in weakly increasing order");
            }

            {
                size_t currentSize = m_data.size();
                if (currentSize <= index) {
                    for (size_t i = currentSize; i <= index; i++)
                        m_data.push_back( m_currentValue );
                }
            }

            m_data[index] = value;
            m_currentValue = value;
            if (m_initialRange == 0)
                m_initialRange = index;

            return change;
        }


        /// Will return the index of the first occurence of @value, or
        /// -1 if @value is not found.
        ssize_t find(const T& value) {
            auto iter = std::find( m_data.begin() , m_data.end() , value);
            if (iter != m_data.end()) {
                // Found normally
                return std::distance( m_data.begin() , iter );
            } else {
                if ((m_data.size() == 0) && (value == m_currentValue))
                    // Not found explicitly - but the value corresponds to the initial 'current value'
                    return 0;

                // Not found
                return -1;
            }
        }


    private:

        void init(T initialValue) {
            m_data.clear();
            m_currentValue = initialValue;
            m_initialValue = initialValue;
            m_initialRange = 0;
        }


        std::vector<T> m_data;
        std::shared_ptr< const TimeMap > m_timeMap;
        T m_currentValue;
        T m_initialValue;
        size_t m_initialRange;
    };
}



#endif
