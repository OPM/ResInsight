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

#ifndef OPM_VALUE_HPP
#define OPM_VALUE_HPP

#include <stdexcept>
#include <string>


/*
  The simple class Value<T> keeps track of a named scalar variable;
  the purpose of this class is to keep strick track of whether the
  value has been assigned or not. Will throw an exception if trying to
  use an unitialized value.
*/



namespace Opm {

template <typename T>
class Value {

private:
    std::string m_name;
    bool m_initialized = false;
    T m_value;


public:

    Value() = default;
    explicit Value(const std::string& name) :
        m_name( name ),
        m_initialized( false )
    { }


    Value(const std::string& name ,T value) :
        m_name( name )
    {
        setValue( value );
    }


    bool hasValue() const {
        return m_initialized;
    }


    T getValue() const {
        if (m_initialized)
            return m_value;
        else
            throw std::logic_error("The value has: " + m_name + " has not been initialized");
    }


    void setValue( T value ) {
        m_initialized = true;
        m_value = value;
    }

    /**
       Will return true if both value instances have been initialized
       to the same value; or none of the values have been
       initialized. Does not compare names.
    */
    bool equal( const Value<T>& other) const {
        if (m_initialized == other.m_initialized) {
            if (m_initialized) {
                if (m_value == other.m_value)
                    return true;  // Have been initialized to same value
                else
                    return false;
            } else
                return true;      // Both undefined
        } else
            return false;
    }

    bool operator==( const Value& rhs ) const {
        return this->equal( rhs );
    }

    bool operator!=( const Value& rhs ) const {
        return !(*this == rhs );
    }


};
}

#endif
