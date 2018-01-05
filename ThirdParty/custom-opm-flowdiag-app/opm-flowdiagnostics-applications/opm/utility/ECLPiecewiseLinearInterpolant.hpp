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

#ifndef OPM_ECLSIMPLE1DINTERPOLANT_HEADER_INCLUDED
#define OPM_ECLSIMPLE1DINTERPOLANT_HEADER_INCLUDED

#include <opm/utility/ECLTableInterpolation1D.hpp>

#include <cassert>
#include <cmath>
#include <exception>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace Opm { namespace Interp1D { namespace PiecewisePolynomial {

    /// Piecewise linear interpolation in set of result columns.
    ///
    /// \tparam Extrapolation Policy class for determining how to
    ///    extrapolate results outside the range covered by the independent
    ///    variate.  Must support member functions \c left and \c right that
    ///    extrapolate a tabulated function to the left and right of the
    ///    input range, respectively.  Typically a policy class from
    ///    namespace ExtrapolationPolicy.
    ///
    /// \tparam IsAscendingRange Flag for whether or not the input range is
    ///    sorted ascendingly or descendingly.  Class \c Linear assumes that
    ///    the input range is sorted, so if \code IsAscendingRange = false
    ///    \endcode, this implies that the input range is treated as being
    ///    sorted descendingly (i.e., as if by \code std::sort(begin, end,
    ///    std::greater<>{}) \endcode.
    template <class Extrapolation, bool IsAscendingRange = true>
    class Linear
    {
    public:
        /// Constructor.
        ///
        /// Essentially a hack.  This creates an invalid interpolant and
        /// exists only to support creating an object backed by an empty
        /// range that will be subsequently discarded and not actually used.
        /// This, in turn, is useful in the construction of live oil/wet gas
        /// property interpolants that have padded tables.
        ///
        /// \param[in] extrap Instance of the configured extrapolation
        ///    policy class.
        explicit Linear(Extrapolation&& extrap)
            : extrap_(std::forward<Extrapolation>(extrap))
            , nCols_ (0)
        {}

        /// Constructor.
        ///
        /// \tparam ElmIterator Iterator over the elements of an input range
        ///    or a result column.  Typically \code
        ///    std::vector<double>::const_iterator \endcode.
        ///
        /// \tparam ValueTransform Representation of a value transformation
        ///    that, often, effects unit conversion of input tables.
        ///    Assumed to implement a function call operator that supports
        ///    the syntax
        ///    \code
        ///       w = ValueTransform(v)
        ///    \endcode
        ///
        /// \param[in] extrap Instance of the configured extrapolation
        ///    policy class.
        ///
        /// \param[in] xBegin Start of range of independent variate.
        ///
        /// \param[in] xEnd One past the end of the range of indpendent
        ///    variate.
        ///
        /// \param[in,out] colIt Sequence of ranges of dependent variates.
        ///    On input, points to beginning of ranges.  On output, each
        ///    range pointer is advanced across \code std::distance(xBegin,
        ///    xEnd) \endcode entries.  This assumes that the underlying
        ///    ranges are formatted according to ECL result set conventions
        ///    (TAB vector from the INIT file).
        ///
        /// \param[in] xTransform Value transformation for the independent
        ///    variate.  Called for each "valid" element of the input range.
        ///
        /// \param[in] colTransform Sequence of value transformations for
        ///    the dependent variates.  In particular, \code
        ///    colTransform[i]() \endcode is invoked on the \c i-th
        ///    dependent variate if and only if the corresponding element of
        ///    the input range is "valid".
        template <class ElmIterator, class ValueTransform>
        Linear(Extrapolation&&                    extrap,
               ElmIterator                        xBegin,
               ElmIterator                        xEnd,
               std::vector<ElmIterator>&          colIt,
               const ValueTransform&              xTransform,
               const std::vector<ValueTransform>& colTransform);

        /// Classify an input point according to the range of the
        /// interpolant's configured independent variate.
        ///
        /// Classification is performed according to the configured policy
        /// for treating the sort order of the input range.
        ///
        /// \param[in] x Input point.
        ///
        /// \return Classification of the input point \p x.
        LocalInterpPoint classifyPoint(const double x) const
        {
            return LocalInterpPoint::identify(this->x_, x, Ascending_P{});
        }

        /// Evaluate interpolant of particular dependent variable at
        /// particular input point.
        ///
        /// \param[in] col Column ID of particular dependent variable.
        ///
        /// \param[in] pt Input point.  Result of previous call to member
        ///    function \code classifyPoint() \endcode.
        ///
        /// \return Value of dependent variable \p col at interpolation
        ///    point \p pt.
        double evaluate(const std::size_t       col,
                        const LocalInterpPoint& pt) const
        {
            if (pt.cat == PointCategory::InRange) {
                // Common case.  Input point is within range of x_.  Placed
                // first to enable early return.
                return this->interpolate(pt, col);
            }

            auto yval = [this](const std::size_t i,
                               const std::size_t j) -> double
            {
                return this->y(i, j);
            };

            if (pt.cat == PointCategory::LeftOfRange) {
                // Extrapolate function ot the left of input range.
                const auto xmin = this->x_.front();

                return this->extrap_.left(this->x_, xmin + pt.t, col, yval);
            }

            assert (pt.cat == PointCategory::RightOfRange);

            // Extrapolate function ot the right of input range.
            const auto xmax = this->x_.back();
            return this->extrap_.right(this->x_, xmax + pt.t, col, yval);
        }

        /// Retrieve abscissas of interpolant's independent variate.
        const std::vector<double>& independentVariable() const
        {
            return this->x_;
        }

        /// Retrieve ordinates of one of the interpolant's dependent
        /// variates.
        ///
        /// \param[in] col Column ID of particular dependent variable.
        ///
        /// \return Ordinates corresponding to particular dependent variate,
        ///    with the \c i-th element matching the \c i-th element of the
        ///    independent variate.
        std::vector<double> resultVariable(const std::size_t col) const
        {
            auto result = std::vector<double>{};

            if (col >= this->nCols_) {
                throw std::domain_error {
                    "Result Column Identifier Ouf of Bounds"
                };
            }

            result.reserve(this->x_.size());
            for (auto n = this->x_.size(), i = 0*n; i < n; ++i) {
                result.push_back(this->y(i, col));
            }

            return result;
        }

    private:
        /// Predicate for ascendingly sorted input ranges.  True_type for
        /// ascending ranges, false_type for descendingly sorted ranges.
        using Ascending_P =
            std::integral_constant<bool, IsAscendingRange>;

        /// Instance of extrapolation policy object.  Invoked when input
        /// points are outside the range of the table's independent variate.
        Extrapolation extrap_;

        /// Number of result columns.
        std::size_t nCols_;

        /// Abscissas (independent variate), compressed to valid points
        /// only.
        std::vector<double> x_;

        /// Ordinates of dependent variates, compressed to valid points of
        /// input range.  Stored with column index (dependent variate ID)
        /// cycling the most rapidly.
        std::vector<double> y_;

        /// Evaluate interpolant of particular dependent variable at
        /// particular input point.
        ///
        /// Implements case of input point being within range of table's
        /// independent variate.
        ///
        /// \param[in] col Column ID of particular dependent variable.
        ///
        /// \param[in] pt Input point.  Result of previous call to member
        ///    function \code classifyPoint() \endcode.
        ///
        /// \return Value of dependent variable \p col at interpolation
        ///    point \p pt.
        double interpolate(const LocalInterpPoint& pt,
                           const std::size_t       col) const
        {
            assert (pt.cat == PointCategory::InRange);
            assert (pt.interval + 1 < this->x_.size());

            const auto left = pt.interval + 0;
            const auto xl   = this->x_[left];
            const auto yl   = this->y (left, col);

            const auto right = pt.interval + 1;
            const auto xr    = this->x_[right];
            const auto yr    = this->y (right, col);

            const auto t = pt.t / (xr - xl);

            return t*yr + (1.0 - t)*yl;
        }

        /// Retrieve value of ordinate at specified row and column pair.
        ///
        /// \param[in] row Row index.
        ///
        /// \param[in] col Column index.
        ///
        /// \return Ordinate at index \code (row, col) \endcode.
        double y(const std::size_t row,
                 const std::size_t col) const
        {
            // Recall: this->y_ stored with column index cycling the most
            // rapidly.

            assert (col < this->nCols_);
            assert (row*this->nCols_ + col < this->y_.size());

            return this->y_[row*this->nCols_ + col];
        }
    };

    template <class Extrapolation, bool IsAscendingRange>
    template <class ElmIterator, class ValueTransform>
    Linear<Extrapolation, IsAscendingRange>::
    Linear(Extrapolation&&                    extrap,
           ElmIterator                        xBegin,
           ElmIterator                        xEnd,
           std::vector<ElmIterator>&          colIt,
           const ValueTransform&              xTransform,
           const std::vector<ValueTransform>& colTransform)
        : extrap_(std::forward<Extrapolation>(extrap))
        , nCols_ (colIt.size())
    {
        // There must be at least one dependent variable/result variable.
        assert (colIt.size() >= 1);

        const auto nRows = std::distance(xBegin, xEnd);

        this->x_.reserve(nRows);
        this->y_.reserve(nRows * colIt.size());

        auto keyValid = [](const double xi)
        {
            // Indep. variable values <= -1.0e20 or >= 1.0e20 signal
            // "unused" table nodes (rows).  These nodes are in the table to
            // fill out the allocated size if one particular sub-table does
            // not use all nodes.  The magic value 1.0e20 is documented in
            // the Fileformats Reference Manual.
            return std::abs(xi) < 1.0e20;
        };

        while (xBegin != xEnd) {
            // Extract relevant portion of the table.  Preallocated rows
            // that are not actually part of the result set (i.e., those
            // that are set to a sentinel value) are discarded.
            if (keyValid(*xBegin)) {
                this->x_.push_back(xTransform(*xBegin));

                auto colID = 0*colTransform.size();
                for (auto ci : colIt) {
                    // Store 'y_' with column index cycling most rapidly.
                    this->y_.push_back(colTransform[colID++](*ci));
                }
            }

            // -------------------------------------------------------------
            // Advance iterators.

            // 1) Independent variable.
            ++xBegin;

            // 2) Dependent/result/columns.
            for (auto& ci : colIt) {
                ++ci;
            }
        }

        // Dispose of any excess capacity.
        if (this->x_.size() < static_cast<decltype(this->x_.size())>(nRows)) {
            this->x_.shrink_to_fit();
            this->y_.shrink_to_fit();
        }

        if (this->x_.size() < 2) {
            // Table has no interval that supports interpolation.  Either
            // just a single node or no nodes at all.  We can't do anything
            // useful here, so don't pretend that this is okay.

            throw std::invalid_argument {
                "No Interpolation Intervals of Non-Zero Size"
            };
        }
    }

}}} // Opm::Interp1D::PiecewisePolynomial

#endif // OPM_ECLSIMPLE1DINTERPOLANT_HEADER_INCLUDED
