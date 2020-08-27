/*
  Copyright 2009, 2010, 2011, 2012 SINTEF ICT, Applied Mathematics.
  Copyright 2009, 2010, 2011, 2012 Statoil ASA.

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

#ifndef OPM_NONUNIFORMTABLELINEAR_HEADER_INCLUDED
#define OPM_NONUNIFORMTABLELINEAR_HEADER_INCLUDED

#include <cmath>
#include <exception>
#include <vector>
#include <utility>

#include <opm/common/ErrorMacros.hpp>
#include <opm/common/utility/numeric/linearInterpolation.hpp>


namespace Opm
{


    /// @brief This class uses linear interpolation to compute the value
    ///        (and its derivative) of a function f sampled at possibly
    ///         nonuniform points. If values outside the domain are sought,
    ///         values will be extrapolated linearly.
    /// @tparam T the range type of the function (should be an algebraic ring type)
    template<typename T>
    class NonuniformTableLinear
    {
    public:
        /// @brief Default constructor.
        NonuniformTableLinear();

        /// @brief Construct from vectors of x and y values.
        /// @param x_values vector of domain values
        /// @param y_values vector of corresponding range values.
        template<class XContainer, class YContainer>
        NonuniformTableLinear(const XContainer& x_values,
                              const YContainer& y_values);

        /// @brief Get the domain.
        /// @return the domain as a pair of doubles.
        std::pair<double, double> domain();

        /// @brief Rescale the domain.
        /// @param new_domain the new domain as a pair of doubles.
        void rescaleDomain(std::pair<double, double> new_domain);

        /// @brief Evaluate the value at x.
        /// @param x a domain value
        /// @return f(x)
        double operator()(const double x) const;

        /// @brief Evaluate the derivative at x.
        /// @param x a domain value
        /// @return f'(x)
        double derivative(const double x) const;

        /// @brief Evaluate the inverse at y. Requires T to be a double.
        /// @param y a range value
        /// @return f^{-1}(y)
        double inverse(const double y) const;

        /// @brief Equality operator.
        /// @param other another NonuniformTableLinear.
        /// @return true if they are represented exactly alike.
        bool operator==(const NonuniformTableLinear& other) const;

    protected:
        std::vector<double> x_values_;
        std::vector<T> y_values_;
        mutable std::vector<T> x_values_reversed_;
        mutable std::vector<T> y_values_reversed_;
    };


    // A utility function
    /// @brief Detect if a sequence is nondecreasing.
    /// @tparam FI a forward iterator whose value type has operator< defined.
    /// @param beg start of sequence
    /// @param end one-beyond-end of sequence
    /// @return false if there exists two consecutive values (v1, v2) in the sequence
    ///         for which v2 < v1, else returns true.
    template <typename FI>
    bool isNondecreasing(const FI beg, const FI end)
    {
        if (beg == end) return true;
        FI it = beg;
        ++it;
        FI prev = beg;
        for (; it != end; ++it, ++prev) {
            if (*it < *prev) {
                return false;
            }
        }
        return true;
    }



    // Member implementations.

    template<typename T>
    inline
    NonuniformTableLinear<T>
    ::NonuniformTableLinear()
    {
    }


    template<typename T>
    template<class XContainer, class YContainer>
    inline
    NonuniformTableLinear<T>
    ::NonuniformTableLinear(const XContainer& x_column,
                            const YContainer& y_column)
        : x_values_( x_column.begin() , x_column.end()),
          y_values_( y_column.begin() , y_column.end())
    {
        assert(isNondecreasing(x_values_.begin(), x_values_.end()));
    }


    template<typename T>
    inline std::pair<double, double>
    NonuniformTableLinear<T>
    ::domain()
    {
        return std::make_pair(x_values_[0], x_values_.back());
    }

    template<typename T>
    inline void
    NonuniformTableLinear<T>
    ::rescaleDomain(std::pair<double, double> new_domain)
    {
        const double a = x_values_[0];
        const double b = x_values_.back();
        const double c = new_domain.first;
        const double d = new_domain.second;
        // x in [a, b] -> x in [c, d]
        for (int i = 0; i < int(x_values_.size()); ++i) {
            x_values_[i] = (x_values_[i] - a)*(d - c)/(b - a) + c;
        }
    }

    template<typename T>
    inline double
    NonuniformTableLinear<T>
    ::operator()(const double x) const
    {
        return Opm::linearInterpolation(x_values_, y_values_, x);
    }

    template<typename T>
    inline double
    NonuniformTableLinear<T>
    ::derivative(const double x) const
    {
        return Opm::linearInterpolationDerivative(x_values_, y_values_, x);
    }

    template<typename T>
    inline double
    NonuniformTableLinear<T>
    ::inverse(const double y) const
    {
        if (y_values_.front() < y_values_.back()) {
            return Opm::linearInterpolation(y_values_, x_values_, y);
        } else {
            if (y_values_reversed_.empty()) {
                y_values_reversed_ = y_values_;
                std::reverse(y_values_reversed_.begin(), y_values_reversed_.end());
                assert(isNondecreasing(y_values_reversed_.begin(), y_values_reversed_.end()));
                x_values_reversed_ = x_values_;
                std::reverse(x_values_reversed_.begin(), x_values_reversed_.end());
            }
            return Opm::linearInterpolation(y_values_reversed_, x_values_reversed_, y);
        }
    }

    template<typename T>
    inline bool
    NonuniformTableLinear<T>
    ::operator==(const NonuniformTableLinear<T>& other) const
    {
        return x_values_ == other.x_values_
            && y_values_ == other.y_values_;
    }

} // namespace Opm

#endif // OPM_NONUNIFORMTABLELINEAR_HEADER_INCLUDED
