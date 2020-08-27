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
#include <vector>
#include <algorithm>
#include <utility>

#include <opm/parser/eclipse/EclipseState/Schedule/TimeMap.hpp>


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



template< class T >
class DynamicState {
    friend class Schedule;
    public:
        typedef typename std::vector< T >::iterator iterator;

        DynamicState() = default;

        DynamicState( const TimeMap& timeMap, T initial ) :
            m_data( timeMap.size(), initial ),
            initial_range( timeMap.size() )
        {}

        DynamicState(const std::vector<T>& data,
                     size_t init_range) :
            m_data(data), initial_range(init_range)
        {}

        void globalReset( T value ) {
            this->m_data.assign( this->m_data.size(), value );
        }

        const T& back() const {
            return m_data.back();
        }

        const T& at( size_t index ) const {
            return this->m_data.at( index );
        }

        const T& operator[](size_t index) const {
            return this->at( index );
        }

        const T& get(size_t index) const {
            return this->at( index );
        }

        void updateInitial( T initial ) {
            std::fill_n( this->m_data.begin(), this->initial_range, initial );
        }


        std::vector<std::pair<std::size_t, T>> unique() const {
            if (this->m_data.empty())
                return {};

            const auto * current_value = std::addressof(this->m_data[0]);
            std::size_t current_index = 0;
            std::vector<std::pair<std::size_t, T>> result{{current_index, *current_value}};
            while (true) {
                if (this->m_data[current_index] != *current_value) {
                    current_value = std::addressof(this->m_data[current_index]);
                    result.emplace_back(current_index, *current_value);
                }

                current_index++;
                if (current_index == this->m_data.size())
                    break;
            }

            return result;
        }

        bool is_new_data(size_t index) const {
            return index == 0 || (at(index) != at(index - 1));
        }

        /**
           If the current value has been changed the method will
           return true, otherwise it will return false.
        */
        bool update( size_t index, T value ) {
            if( this->initial_range == this->m_data.size() )
                this->initial_range = index;

            const bool change = (value != this->m_data.at( index ));

            if( !change ) return false;

            std::fill( this->m_data.begin() + index,
                       this->m_data.end(),
                       value );

            return true;
        }

        void update_elm( size_t index, const T& value ) {
            if (this->m_data.size() <= index)
                throw std::out_of_range("Invalid index for update_elm()");

            this->m_data[index] = value;
        }


    /*
      Will assign all currently equal values starting at index with the new
      value. Purpose of the method is to support manipulations of an existing
      Schedule object, if e.g. a well is initially closed in the interval
      [T1,T2] and then opened at time T1 < Tx < T2 then the open should be
      applied for all times in the range [Tx,T2].
    */
    void update_equal(size_t index, const T& value) {
        if (this->m_data.size() <= index)
            throw std::out_of_range("Invalid index for update_equal()");

        const T prev_value = this->m_data[index];
        if (prev_value == value)
            return;

        while (true) {
            if (this->m_data[index] != prev_value)
                break;

            this->m_data[index] = value;

            index++;
            if (index == this->m_data.size())
                break;

        }
    }

    /// Will return the index of the first occurence of @value, or
    /// -1 if @value is not found.
    int find(const T& value) const {
        auto iter = std::find( m_data.begin() , m_data.end() , value);
        if( iter == this->m_data.end() ) return -1;

        return std::distance( m_data.begin() , iter );
    }

    template<typename P>
    int find_if(P&& pred) const {
        auto iter = std::find_if(m_data.begin(), m_data.end(), std::forward<P>(pred));
        if( iter == this->m_data.end() ) return -1;

        return std::distance( m_data.begin() , iter );
    }

    /// Will return the index of the first value which is != @value, or -1
    /// if all values are == @value
    int find_not(const T& value) const {
        auto iter = std::find_if_not( m_data.begin() , m_data.end() , [&value] (const T& elm) { return value == elm; });
        if( iter == this->m_data.end() ) return -1;

        return std::distance( m_data.begin() , iter );
    }

    iterator begin() {
        return this->m_data.begin();
    }


    iterator end() {
        return this->m_data.end();
    }


    std::size_t size() const {
        return this->m_data.size();
    }

    const std::vector<T>& data() const {
        return m_data;
    }

    size_t initialRange() const {
        return initial_range;
    }

    bool operator==(const DynamicState<T>& data) const {
        return m_data == data.m_data &&
               initial_range == data.initial_range;
    }

    // complexType=true if contained type has a serializeOp
    template<class Serializer, bool complexType = true>
    void serializeOp(Serializer& serializer)
    {
        std::vector<T> unique;
        auto indices = split(unique);
        serializer.template vector<T,complexType>(unique);
        serializer(indices);
        if (!serializer.isSerializing())
            reconstruct(unique, indices);
    }

    private:
        std::vector< T > m_data;
        size_t initial_range;

        std::vector<size_t> split(std::vector<T>& unique) const {
            std::vector<size_t> idxVec;
            idxVec.reserve(m_data.size() + 1);
            for (const auto& w : m_data) {
                auto candidate = std::find(unique.begin(), unique.end(), w);
                size_t idx = candidate - unique.begin();
                if (candidate == unique.end()) {
                    unique.push_back(w);
                    idx = unique.size() - 1;
                }
                idxVec.push_back(idx);
            }
            idxVec.push_back(initial_range);

            return idxVec;
        }

        void reconstruct(const std::vector<T>& unique,
                         const std::vector<size_t>& idxVec) {
            m_data.clear();
            m_data.reserve(idxVec.size() - 1);
            for (size_t i = 0; i < idxVec.size() - 1; ++i)
                m_data.push_back(unique[idxVec[i]]);

            initial_range = idxVec.back();
        }
};

}

#endif

