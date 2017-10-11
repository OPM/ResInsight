/*
  Copyright 2017 Statoil ASA.

  This file is part of the Open Porous Media Project (OPM).

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

#include <opm/utility/ECLTableInterpolation1D.hpp>

#include <algorithm>
#include <cassert>
#include <exception>
#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace Details {
    template <class Compare>
    std::vector<double>::size_type
    intervalInRange(const std::vector<double>& abscissas,
                    const double               x,
                    Compare&&                  compare)
    {
        // Verify that 'x' is within range.  Order of arguments matters.
        assert (! compare(x, abscissas.front()));
        assert (! compare(abscissas.back(), x));

        const auto b = std::begin(abscissas);
        const auto p =
            std::lower_bound(b, std::end(abscissas), x,
                             std::forward<Compare>(compare));

        assert (p != std::end(abscissas));

        // p = lower_bound() => p identifies *right-hand* (upper) end-point
        // of interval (insertion point) => (p - b) is index of right-hand
        // end-point.  Consequently (p - b) - 1 is index of *left-hand*
        // end-point and the point with which we associate the pertinent
        // interval.  Special case handling for p == b.

        return (p == b) ? 0 : (p - b) - 1;
    }

    template <class Compare>
    Opm::Interp1D::PiecewisePolynomial::LocalInterpPoint
    identifyPoint(const std::vector<double>& xi,
                  const double               x,
                  Compare&&                  compare)
    {
        namespace PP = ::Opm::Interp1D::PiecewisePolynomial;
        using PCat   = ::Opm::Interp1D::PointCategory;

        const auto left = xi.front();
        if (compare(x, left)) {
            // Left of min(xi) (== xi.front())
            return PP::LocalInterpPoint {
                PCat::LeftOfRange, 0, x - left,
            };
        }

        const auto right = xi.back();
        if (compare(right, x)) {
            // Right of max(xi) (== xi.back())
            return PP::LocalInterpPoint {
                PCat::RightOfRange, xi.size() - 1, x - right,
            };
        }

        // Common case: x \in [min(xi), max(xi)]
        const auto interval = intervalInRange(xi, x, compare);

        assert ((interval + 1 < xi.size()) || (xi.size() == 1));

        return PP::LocalInterpPoint {
            PCat::InRange, interval, x - xi[interval],
        };
    }

    template <class Compare = std::less<double>>
    Opm::Interp1D::PiecewisePolynomial::LocalInterpPoint
    identify(const std::vector<double>& xi,
             const double               x,
             Compare&&                  compare = Compare{})
    {
        if (xi.empty()) {
            throw std::invalid_argument {
                "Cannot Relate Point to Non-Existent Range of Variable"
            };
        }

        return identifyPoint(xi, x, std::forward<Compare>(compare));
    }
} // Anonymous

// =====================================================================

Opm::Interp1D::PiecewisePolynomial::LocalInterpPoint
Opm::Interp1D::PiecewisePolynomial::LocalInterpPoint::
identify(const std::vector<double>& xi,
         const double               x, std::true_type)
{
    return Details::identify(xi, x);
}

Opm::Interp1D::PiecewisePolynomial::LocalInterpPoint
Opm::Interp1D::PiecewisePolynomial::LocalInterpPoint::
identify(const std::vector<double>& xi,
         const double               x, std::false_type)
{
    return Details::identify(xi, x, std::greater<double>{});
}
