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

#ifndef OPM_ECLTABLEINTERPOLATION1D_HEADER_INCLUDED
#define OPM_ECLTABLEINTERPOLATION1D_HEADER_INCLUDED

#include <cassert>
#include <type_traits>
#include <vector>

/// \file
///
/// Common types and policies for one-dimensional (single variate)
/// interpolation of tabulated functions.  Mainly intended to support
/// piecewise linear interpolation.

namespace Opm { namespace Interp1D {

    /// Categories of interpolation behaviour according to location of
    /// interpolation point with respect to range of independent variable.
    enum class PointCategory {
        /// Point within range of independent variable.
        InRange,

        /// Point is to the left of range.
        LeftOfRange,

        /// Point is to the right of range.
        RightOfRange,
    };

    /// Functionality for interpolating functions of a single variate using
    /// piecewise polynomials.
    namespace PiecewisePolynomial {

        /// Policies that implement particular behaviours for extrapolating
        /// a tabulated function, assuming ECL's table format, outside the
        /// tabulated range of its independent variable.
        namespace ExtrapolationPolicy {
            /// Extrapolate function using constant values.
            class Constant
            {
            public:
                /// Extrapolate function to the left of range using first
                /// tabulated function value.
                ///
                /// \tparam TabulatedFunction Representation of a tabulated
                ///    function.  Must support function call operator such
                ///    that the statement \code y = Func(i, col); \endcode
                ///    returns the value of the \c col-th dependent variate
                ///    at the \c i-th abscissa.
                ///
                /// \tparam Index Integer type representing an index into
                ///    the abscissas of the tabulated function.
                ///
                /// \param[in] xmin Location of left-most abscissa.  Unused.
                ///
                /// \param[in] x Interpolation point (global coordinates).
                ///    Unused.
                ///
                /// \param[in] col Column index identifying which dependent
                ///    variate to extrapolate.
                ///
                /// \param[in] f Tabulated function instance.
                ///
                /// \return Value of \p col-th variate of tabulated function
                ///    extrapolated to the left of the range of table's
                ///    independent variate.
                template <class TabulatedFunction, class Index>
                double left(const std::vector<double>& /* xi */,
                            const double               /* x */,
                            const Index                col ,
                            TabulatedFunction&&        f) const
                {
                    return f(0, col);
                }

                /// Extrapolate function to the rigth of range using "last"
                /// tabulated function value.
                ///
                /// \tparam TabulatedFunction Representation of a tabulated
                ///    function.  Must support function call operator such
                ///    that the statement \code y = Func(i, col); \endcode
                ///    returns the value of the \c col-th dependent variate
                ///    at the \c i-th abscissa.
                ///
                /// \tparam Index Integer type representing an index into
                ///    the abscissas of the tabulated function.
                ///
                /// \param[in] xi Location of tabulated function's abscissas
                ///    (unused).
                ///
                /// \param[in] x Interpolation point (global coordinates).
                ///    Unused.
                ///
                /// \param[in] col Column index identifying which dependent
                ///    variate to extrapolate.
                ///
                /// \param[in] f Tabulated function instance.
                ///
                /// \return Value of \p col-th variate of tabulated function
                ///    extrapolated to the rigth of the range of table's
                ///    independent variate.
                template <class TabulatedFunction, class Index>
                double right(const std::vector<double>& xi,
                             const double               /* x */,
                             const Index                col,
                             TabulatedFunction&&        f) const
                {
                    return f(xi.size() - 1, col);
                }
            };

            /// Extrapolate function using extrapolated/estimated
            /// derivatives in range end-points.
            ///
            /// Derivatives estimated from table points nearest to the end
            /// of the range.
            class Linearly
            {
            public:
                /// Extrapolate function to the left of range using first
                /// tabulated function value and estimated derivative.
                ///
                /// \tparam TabulatedFunction Representation of a tabulated
                ///    function.  Must support function call operator such
                ///    that the statement \code y = Func(i, col); \endcode
                ///    returns the value of the \c col-th dependent variate
                ///    at the \c i-th abscissa.
                ///
                /// \tparam Index Integer type representing an index into
                ///    the abscissas of the tabulated function.
                ///
                /// \param[in] xi Location of tabulated function's abscissas.
                ///
                /// \param[in] x Interpolation point (global coordinates).
                ///
                /// \param[in] col Column index identifying which dependent
                ///    variate to extrapolate.
                ///
                /// \param[in] f Tabulated function instance.
                ///
                /// \return Value of \p col-th variate of tabulated function
                ///    extrapolated to the left of the range of table's
                ///    independent variate.
                template <class TabulatedFunction, class Index>
                double left(const std::vector<double>& xi,
                            const double               x,
                            const Index                col,
                            TabulatedFunction&&        f) const
                {
                    // Derivative of f(i,col)
                    const auto f0   = f(0, col);
                    const auto f1   = f(1, col);
                    const auto dfdx = (f1 - f0) / (xi[1] - xi[0]);

                    // <= 0 if ascending range.
                    const auto dx   = x - xi.front();

                    return f0 + (dfdx * dx);
                }

                /// Extrapolate function to the rigth of range using "last"
                /// tabulated function value and estimated derivative.
                ///
                /// \tparam TabulatedFunction Representation of a tabulated
                ///    function.  Must support function call operator such
                ///    that the statement \code y = Func(i, col); \endcode
                ///    returns the value of the \c col-th dependent variate
                ///    at the \c i-th abscissa.
                ///
                /// \tparam Index Integer type representing an index into
                ///    the abscissas of the tabulated function.
                ///
                /// \param[in] xi Location of tabulated function's abscissas.
                ///
                /// \param[in] x Interpolation point (global coordinates).
                ///
                /// \param[in] col Column index identifying which dependent
                ///    variate to extrapolate.
                ///
                /// \param[in] f Tabulated function instance.
                ///
                /// \return Value of \p col-th variate of tabulated function
                ///    extrapolated to the rigth of the range of table's
                ///    independent variate.
                template <class TabulatedFunction, class Index>
                double right(const std::vector<double>& xi,
                             const double               x,
                             const Index                col,
                             TabulatedFunction&&        f) const
                {
                    const auto nRows = xi.size();

                    // Derivative of f(i,col)
                    const auto f0   = f(nRows - 2, col);
                    const auto f1   = f(nRows - 1, col);
                    const auto dfdx =
                        (f1 - f0) / (xi[nRows - 1] - xi[nRows - 2]);

                    // >= 0 if ascending range
                    const auto dx = x - xi.back();

                    return f1 + (dfdx * dx);
                }
            };

            /// Extrapolate function using tabulated constant derivatives in
            /// range end-points.
            class LinearlyWithDerivatives
            {
            public:
                /// Constructor
                ///
                /// \param[in] nResCol Number of function value (result)
                ///    columns in underlying tabulated function.  Equal to
                ///    total number of dependent variable columns less the
                ///    number of derivative columns.  Typically two as in
                ///    the cases of the PV{D,T}{G,O} tables.
                explicit LinearlyWithDerivatives(const std::size_t nResCol)
                    : nResCol_(nResCol)
                {}

                /// Extrapolate function to the left of range using first
                /// tabulated function value and corresponding derivative.
                ///
                /// \tparam TabulatedFunction Representation of a tabulated
                ///    function.  Must support function call operator such
                ///    that the statement \code y = Func(i, col); \endcode
                ///    returns the value of the \c col-th dependent variate
                ///    at the \c i-th abscissa.
                ///
                /// \tparam Index Integer type representing an index into
                ///    the abscissas of the tabulated function.
                ///
                /// \param[in] xi Location of tabulated function's abscissas.
                ///
                /// \param[in] x Interpolation point (global coordinates).
                ///
                /// \param[in] col Column index identifying which dependent
                ///    variate to extrapolate.
                ///
                /// \param[in] f Tabulated function instance.
                ///
                /// \return Value of \p col-th variate of tabulated function
                ///    extrapolated to the left of the range of table's
                ///    independent variate.
                template <class TabulatedFunction, class Index>
                double left(const std::vector<double>& xi,
                            const double               x,
                            const Index                col,
                            TabulatedFunction&&        f) const
                {
                    // Derivative of f(i,col) in f(i, nResCol_ + col)
                    const auto dfdx = f(0, this->nResCol_ + col);
                    const auto dx   = x - xi.front(); // <= 0 if ascending range

                    return f(0, col) + (dfdx * dx);
                }

                /// Extrapolate function to the rigth of range using "last"
                /// tabulated function value and corresponding derivative.
                ///
                /// \tparam TabulatedFunction Representation of a tabulated
                ///    function.  Must support function call operator such
                ///    that the statement \code y = Func(i, col); \endcode
                ///    returns the value of the \c col-th dependent variate
                ///    at the \c i-th abscissa.
                ///
                /// \tparam Index Integer type representing an index into
                ///    the abscissas of the tabulated function.
                ///
                /// \param[in] xi Location of tabulated function's abscissas.
                ///
                /// \param[in] x Interpolation point (global coordinates).
                ///
                /// \param[in] col Column index identifying which dependent
                ///    variate to extrapolate.
                ///
                /// \param[in] nRows Number of rows (abscissas) in
                ///    function's underlying table representation.
                ///
                /// \param[in] f Tabulated function instance.
                ///
                /// \return Value of \p col-th variate of tabulated function
                ///    extrapolated to the rigth of the range of table's
                ///    independent variate.
                template <class TabulatedFunction, class Index>
                double right(const std::vector<double>& xi,
                             const double               x,
                             const Index                col,
                             TabulatedFunction&&        f) const
                {
                    const auto nRows = xi.size();

                    // Derivative of f(i,col) in f(i, nResCol_ + col)
                    const auto dfdx = f(nRows - 1, this->nResCol_ + col);
                    const auto dx   = x - xi.back(); // >= 0 if ascending range

                    return f(nRows - 1, col) + (dfdx * dx);
                }

            private:
                /// Number of function value (result) columns in underlying
                /// tabulated function.
                std::size_t nResCol_;
            };
        } // ExtrapolationPolicy

        /// Interpolation point localized with respect to sequence of
        /// non-overlapping intervals with separating abscissas.
        struct LocalInterpPoint {
            /// Interpolation behaviour of point with respect to range.
            PointCategory cat;

            /// Interval index.  Meaningful only with respect to particular
            /// sequence of abscissas.  Zero when
            ///
            /// \code
            ///    cat == PointCategory::LeftOfRange
            /// \endcode
            ///
            /// Equal to number of abscissas (i.e., one greater than number
            /// of intervals) when
            ///
            /// \code
            ///    cat == PointCategory::RightOfRange
            /// \endcode.
            std::vector<double>::size_type interval;

            /// Local coordinate within interval.  Defined as
            ///
            /// \code
            ///    x - abscissa[interval]
            /// \endcode
            ///
            /// Non-positive for PointCategory::LeftOfRange.  Non-negative
            /// otherwise.
            double t;

            /// Identify point category and, usually, particular interval in
            /// which a specific point is localized.
            ///
            /// Overload for usual case of ascendingly sorted abscissas.
            ///
            /// \param[in] xi Sequence of separating abscissas representing
            ///    non-overlapping intervals of an independent variable.
            ///
            /// \param[in] x Sample point.
            ///
            /// \param[in] is_ascending Tagged dispatch overload
            ///    disambiguation object.  Unused.
            ///
            /// \return Sample point localized with respect to the abscissas
            ///    \p xi.
            static LocalInterpPoint
            identify(const std::vector<double>& xi,
                     const double               x,
                     std::true_type is_ascending = std::true_type{});

            /// Identify point category and, usually, particular interval in
            /// which a specific point is localized.
            ///
            /// Overload for case of descendingly sorted abscissas (e.g.,
            /// through \code std::sort(first, last, std::greater<>{})
            /// \endcode).
            ///
            /// \param[in] xi Sequence of separating abscissas representing
            ///    non-overlapping intervals of an independent variable.
            ///
            /// \param[in] x Sample point.
            ///
            /// \param[in] is_ascending Tagged dispatch overload
            ///    disambiguation object.  Unused.
            ///
            /// \return Sample point localized with respect to the abscissas
            ///    \p xi.
            static LocalInterpPoint
            identify(const std::vector<double>& xi,
                     const double               x,
                     std::false_type is_ascending);
        };

    } // PiecewisePolynomial

}} // Opm::Interp1D

#endif // OPM_ECLTABLEINTERPOLATION1D_HEADER_INCLUDED
