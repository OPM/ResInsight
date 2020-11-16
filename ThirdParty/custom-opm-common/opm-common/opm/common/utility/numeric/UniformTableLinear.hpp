/*
  Copyright 2010 SINTEF ICT, Applied Mathematics.

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

#ifndef OPM_UNIFORMTABLELINEAR_HEADER_INCLUDED
#define OPM_UNIFORMTABLELINEAR_HEADER_INCLUDED

#include <cmath>
#include <exception>
#include <vector>
#include <utility>
#include <iostream>

#include <opm/common/ErrorMacros.hpp>

namespace Opm {

	/// @brief This class uses linear interpolation to compute the value
	///        (and its derivative) of a function f sampled at uniform points.
	/// @tparam T the range type of the function (should be an algebraic ring type)
	template<typename T>
	class UniformTableLinear
	{
	public:
	    /// @brief Default constructor.
	    UniformTableLinear();

	    /// @brief Construct from vector of y-values.
	    /// @param xmin the x value corresponding to the first y value.
	    /// @param xmax the x value corresponding to the last y value.
	    /// @param y_values vector of range values.
	    UniformTableLinear(double xmin,
                               double xmax,
                               const std::vector<T>& y_values);

	    /// @brief Construct from array of y-values.
	    /// @param xmin the x value corresponding to the first y value.
	    /// @param xmax the x value corresponding to the last y value.
	    /// @param y_values array of range values.
            /// @param num_y_values the number of values in y_values.
	    UniformTableLinear(double xmin,
                               double xmax,
                               const T* y_values,
                               int num_y_values);

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

	    /// @brief Equality operator.
	    /// @param other another UniformTableLinear.
	    /// @return true if they are represented exactly alike.
	    bool operator==(const UniformTableLinear& other) const;

	    /// @brief Policies for how to behave when trying to evaluate outside the domain.
	    enum RangePolicy {Throw = 0, ClosestValue = 1, Extrapolate = 2};

	    /// @brief Sets the behavioural policy for evaluation to the left of the domain.
	    /// @param rp the policy
	    void setLeftPolicy(RangePolicy rp);

	    /// @brief Sets the behavioural policy for evaluation to the right of the domain.
	    /// @param rp the policy
	    void setRightPolicy(RangePolicy rp);

	protected:
            double xmin_;
            double xmax_;
            double xdelta_;
	    std::vector<T> y_values_;
	    RangePolicy left_;
	    RangePolicy right_;
            template <typename U>
            friend std::ostream& operator<<(std::ostream& os, const UniformTableLinear<U>& t);
	};


	// Member implementations.

	template<typename T>
	inline
	UniformTableLinear<T>
	::UniformTableLinear()
	    : left_(ClosestValue), right_(ClosestValue)
	{
	}

	template<typename T>
	inline
	UniformTableLinear<T>
	::UniformTableLinear(double xmin,
                             double xmax,
                             const std::vector<T>& y_values)
	    : xmin_(xmin), xmax_(xmax), y_values_(y_values),
	      left_(ClosestValue), right_(ClosestValue)
	{
            assert(xmax > xmin);
            assert(y_values.size() > 1);
            xdelta_ = (xmax - xmin)/(y_values.size() - 1);
	}

	template<typename T>
	inline
	UniformTableLinear<T>
	::UniformTableLinear(double xmin,
                             double xmax,
                             const T* y_values,
                             int num_y_values)
	    : xmin_(xmin), xmax_(xmax),
              y_values_(y_values, y_values + num_y_values),
	      left_(ClosestValue), right_(ClosestValue)
	{
            assert(xmax > xmin);
            assert(y_values_.size() > 1);
            xdelta_ = (xmax - xmin)/(y_values_.size() - 1);
	}

	template<typename T>
	inline std::pair<double, double>
	UniformTableLinear<T>
	::domain()
	{
	    return std::make_pair(xmin_, xmax_);
	}

	template<typename T>
	inline void
	UniformTableLinear<T>
	::rescaleDomain(std::pair<double, double> new_domain)
	{
            xmin_ = new_domain.first;
            xmax_ = new_domain.second;
            xdelta_ = (xmax_ - xmin_)/(y_values_.size() - 1);
	}

	template<typename T>
	inline double
	UniformTableLinear<T>
	::operator()(const double xparam) const
	{
            // Implements ClosestValue policy.
            double x = std::min(xparam, xmax_);
            x = std::max(x, xmin_);

            // Lookup is easy since we are uniform in x.
            double pos = (x - xmin_)/xdelta_;
            double posi = std::floor(pos);
            int left = int(posi);
            if (left == int(y_values_.size()) - 1) {
                // We are at xmax_
                return y_values_.back();
            }
            double w = pos - posi;
	    return (1.0 - w)*y_values_[left] + w*y_values_[left + 1];
	}

	template<typename T>
	inline double
	UniformTableLinear<T>
	::derivative(const double xparam) const
	{
            // Implements derivative consistent
            // with ClosestValue policy for function
            double value;
            if (xparam > xmax_ || xparam < xmin_) {
                value = 0.0;
            } else {
                double x = std::min(xparam, xmax_);
                x =  std::max(x, xmin_);
                // Lookup is easy since we are uniform in x.
                double pos = (x - xmin_)/xdelta_;
                double posi = std::floor(pos);
                int left = int(posi);
                if (left == int(y_values_.size()) - 1) {
                    // We are at xmax_
                    --left;
                }
                value = (y_values_[left + 1] - y_values_[left])/xdelta_;
            }
            return value;
	}


	template<typename T>
	inline bool
	UniformTableLinear<T>
	::operator==(const UniformTableLinear<T>& other) const
	{
	    return xmin_ == other.xmin_
                && xdelta_ == other.xdelta_
		&& y_values_ == other.y_values_
		&& left_ == other.left_
		&& right_ == other.right_;
	}

	template<typename T>
	inline void
	UniformTableLinear<T>
	::setLeftPolicy(RangePolicy rp)
	{
	    if (rp != ClosestValue) {
		OPM_THROW(std::runtime_error, "Only ClosestValue RangePolicy implemented.");
	    }
	    left_ = rp;
	}

	template<typename T>
	inline void
	UniformTableLinear<T>
	::setRightPolicy(RangePolicy rp)
	{
	    if (rp != ClosestValue) {
		OPM_THROW(std::runtime_error, "Only ClosestValue RangePolicy implemented.");
	    }
	    right_ = rp;
	}


        template <typename T>
        inline std::ostream& operator<<(std::ostream& os, const UniformTableLinear<T>& t)
        {
            int n = t.y_values_.size();
            for (int i = 0; i < n; ++i) {
                double f = double(i)/double(n - 1);
                os << (1.0 - f)*t.xmin_ + f*t.xmax_
                   << "   " << t.y_values_[i] << '\n';
            }
            return os;
        }

    namespace utils
    {
        using Opm::UniformTableLinear;
    }


} // namespace Opm

#endif // OPM_UNIFORMTABLELINEAR_HEADER_INCLUDED
