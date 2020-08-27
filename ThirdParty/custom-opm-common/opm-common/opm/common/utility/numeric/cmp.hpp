/*
  Copyright 2016 Statoil ASA.

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

#ifndef COMMON_UTIL_NUMERIC_CMP
#define COMMON_UTIL_NUMERIC_CMP

#include <cstddef>
#include <vector>
#include <type_traits>
#include <cmath>

// Make sure Windows macro definitions are
// undefined here, to be able to use std::min/max.
#undef max
#undef min

#include <algorithm>

namespace Opm {

    /// In the namespace cmp are implemented functions for
    /// approximate comparison of double values based on absolute
    /// and relative difference. There are three functions:
    ///
    ///   scalar_equal<T>() : Compare two <T> values.
    ///
    ///   ptr_equal<T>(): This compares all the element in the
    ///      two T * pointers.
    ///
    ///   vector_equal<T>(): This compares all the elements in
    ///      two std::vector<T> instances.
    ///
    /// For both vector_equal<T>() and ptr_equal<T>() the
    /// actual comparison is based on the scalar_equal<T>()
    /// function. All functions exist as two overloads, one which
    /// takes explicit input values for the absolute and relative
    /// epsilon, and one which uses default values.  
    ///
    /// The comparison functions are implemented as templates, with
    /// the following caveats:
    ///
    ///  1. The static_assert() in scalar_equal<T> ensures that only
    ///     floating point types can be used.
    ///
    ///  2. The default epsilon values are of type double -
    ///     irrespective of the type of data being compared.
    ///
    /// For more details of floating point comparison please consult
    /// this reference:
    ///
    ///    https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/

    namespace cmp {

        const double default_abs_epsilon = 1e-8;
        const double default_rel_epsilon = 1e-5;

        template<typename T>
        bool scalar_equal(T value1, T value2, T abs_eps , T rel_eps) {
            static_assert(std::is_floating_point<T>::value, "Function scalar_equal() A can only be instantiated with floating point types");

	    bool equal = true;
            T diff = std::fabs(value1 - value2);
            if (diff > abs_eps) {
                T scale = std::max(std::fabs(value1), std::fabs(value2));

                if (diff > scale * rel_eps) {
                    equal = false;
		}
	    }
            return equal;
	}


        template<typename T>
        bool scalar_equal(T value1, T value2) {
  	    return scalar_equal<T>( value1 , value2 , default_abs_epsilon , default_rel_epsilon );
	}

        template<typename T>
        bool vector_equal(const std::vector<T>& v1, const std::vector<T>& v2, T abs_eps, T rel_eps) {
          if (v1.size() != v2.size()) {
                return false;
            }

            for (size_t i = 0; i < v1.size(); i++) {
                if (!scalar_equal<T>( v1[i], v2[i], abs_eps, rel_eps ))
                    return false;
            }

            return true;
        }

        template<typename T>
        bool vector_equal(const std::vector<T>& v1, const std::vector<T>& v2) {
            return vector_equal<T>(v1, v2, default_abs_epsilon, default_rel_epsilon);
        }


        template<typename T>
        bool array_equal(const T* p1, const T* p2, size_t num_elements, T abs_eps, T rel_eps) {
            if (memcmp(p1 , p2 , num_elements * sizeof * p1) == 0)
                return true;
            else {
                size_t index;
                for (index = 0; index < num_elements; index++) {
                    if (!scalar_equal<T>( p1[index] , p2[index] , abs_eps , rel_eps)) {
                        return false;
                    }
                }
            }
            return true;
        }


        template<typename T>
        bool array_equal(const T* p1, const T* p2, size_t num_elements) {
             return array_equal<T>(p1, p2, num_elements , default_abs_epsilon, default_rel_epsilon);
        }
    }
}

#endif
