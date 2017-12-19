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

#ifndef OPM_ECLPROPTABLE_HEADER_INCLUDED
#define OPM_ECLPROPTABLE_HEADER_INCLUDED

#include <opm/utility/ECLPiecewiseLinearInterpolant.hpp>
#include <opm/utility/ECLTableInterpolation1D.hpp>

#include <functional>
#include <vector>

/// \file
///
/// ECL Tabulated Functions (e.g., saturation functions).

namespace Opm {

    /// Raw table data from which to construct collection of interpolants.
    struct ECLPropTableRawData
    {
        /// Representation of the raw table data.  1D array with implicit
        /// substructure.
        using DataVector = std::vector<double>;

        /// Size type for subsets of table data.
        using SizeType = DataVector::size_type;

        /// Iterator to table elements.  Must be random access.
        using ElementIterator = DataVector::const_iterator;

        /// Raw table data.  Column major (Fortran) order.  Typically
        /// copied/extracted directly from TAB vector of INIT result-set.
        /// Array of size \code numRows * numCols * numPrimary \endcode for
        /// each table, stored consecutively.
        DataVector data{};

        /// Primary lookup key for 2D interpolation.  Only relevant for PVT
        /// tables of wet gas and/or live oil (Pg or Rs, respectively).
        /// Array of size \c numPrimary elements for each table, stored
        /// consecutively.
        DataVector primaryKey{};

        /// Number of primary key elements for each individual table.  Only
        /// relevant (i.e., != 1) for PVT tables of wet gas and/or live oil.
        SizeType numPrimary{0};

        /// Number of rows allocated in the result set for each individual
        /// primary key.  Typically corresponds to setting in one of the
        /// *DIMS keywords.  Should normally be at least two for saturation
        /// functions.
        SizeType numRows{0};

        /// Number of columns in this table.  Varies by keyword/table.
        SizeType numCols{0};

        /// Number of tables of this type.  Must match the corresponding
        /// region keyword.
        SizeType numTables{0};
    };

    /// Build a sequence of table interpolants from raw tabulated data,
    /// assuming table conventions in the INIT file's TABDIMS/TAB vectors.
    ///
    /// \tparam Interpolant Representation of a table interpolant.
    template <class Interpolant>
    struct MakeInterpolants
    {
        /// Create sequence of table interpolants.
        ///
        /// This function is aware of the internal layout of the INIT file's
        /// tabulated function and knows how to identify table data ranges
        /// corresponding to a single table.  In particular we know how to
        /// the data according to region IDs and how to apply further
        /// partitioning according to primary lookup keys (e.g., for RS
        /// nodes in PVTO tables).
        ///
        /// \tparam Factory Interpolant construction function.  Usually a
        ///    class constructor wrapped in a lambda.  The call
        ///    \code
        ///      I = Factory(xBegin, xEnd, colIt);
        ///    \endcode
        ///    must construct an instance \c I of type \p Interpolant.
        ///    Here, \c xBegin and \c xEnd demarcate the range of a single
        ///    table's independent variate and \c colIt are column iterators
        ///    positioned at the beginning of each of the table's dependent
        ///    (result) column.
        ///
        ///    Note: The construction function is expected to advance each
        ///    column iterator across \code distance(xBegin, xEnd) \endcode
        ///    entries.
        ///
        /// \param[in] raw Raw tabulated data.  Must correspond to a single
        ///    table vector, e.g. the SWFN data.
        ///
        /// \param[in] construct Callback function that knows how to build a
        ///    single interpolant given a sequence ranges of of independent
        ///    and dependent tabulated function values.  Must advance the
        ///    dependent column iterators and perform appropriate unit
        ///    conversion on the table data if needed (e.g., for capillary
        ///    pressure data or viscosity values).
        template <class Factory>
        static std::vector<Interpolant>
        fromRawData(const ECLPropTableRawData& raw,
                    Factory&&                  construct)
        {
            auto interp = std::vector<Interpolant>{};

            const auto numInterp = raw.numTables * raw.numPrimary;

            // Table format: numRows*numInterp values of first column
            // (indep. var) followed by numCols-1 dependent variable
            // (function value result) columns of numRows*numInterp values
            // each, one column at a time.
            const auto colStride = raw.numRows * numInterp;

            // Position column iterators (independent variable and results
            // respectively) at beginning of each pertinent table column.
            auto xBegin = std::begin(raw.data);
            auto colIt  = std::vector<decltype(xBegin)> {
                xBegin + colStride
            };

            for (auto col = 0*raw.numCols + 1;
                      col <   raw.numCols - 1; ++col)
            {
                colIt.push_back(colIt.back() + colStride);
            }

            // Construct actual interpolants by invoking the
            // constructor/factory function on each sub-table.
            for (auto i = 0*numInterp;
                      i <   numInterp; ++i, xBegin += raw.numRows)
            {
                auto xEnd = xBegin + raw.numRows;

                // Layering violation:
                //    The constructor is expected to advance the result
                //    column iterators across 'numRows' entries.
                interp.push_back(construct(xBegin, xEnd, colIt));
            }

            return interp;
        }
    };

    /// Collection of 1D interpolants from tabulated functions (e.g., the
    /// saturation functions).
    class SatFuncInterpolant
    {
    public:
        /// Protocol for converting raw table input data to strict SI unit
        /// conventions.
        struct ConvertUnits
        {
            /// Convenience type alias for a value transformation.
            using Converter = std::function<double(const double)>;

            /// How to convert the independent variate (1st column)
            Converter indep;

            /// How to convert the dependent variates (2nd... columns).
            std::vector<Converter> column;
        };

        /// Constructor.
        ///
        /// \param[in] raw Raw table data for this collection.
        ///
        /// \param[in] convert Unit conversion support.  Mostly applicable
        ///    to capillary pressure.  Assumed to convert raw table data to
        ///    strict SI unit conventions.
        SatFuncInterpolant(const ECLPropTableRawData& raw,
                           const ConvertUnits&        convert);

        /// Wrapper type to disambiguate API usage.  Represents a table ID.
        struct InTable {
            /// Table ID.
            ECLPropTableRawData::SizeType i;
        };

        /// Wrapper type to disambiguate API usage.  Represents a column ID.
        struct ResultColumn {
            /// Column ID.
            ECLPropTableRawData::SizeType i;
        };

        /// Evaluate 1D interpolant in sequence of points.
        ///
        /// \param[in] t ID of sub-table of interpolant.
        ///
        /// \param[in] c ID of result column/dependent variable.
        ///
        /// \param[in] x Points at which to evaluate interpolant.
        ///
        /// \return Function values of dependent variable \p c evaluated at
        ///    points \p x in table \p t.
        std::vector<double>
        interpolate(const InTable&             t,
                    const ResultColumn&        c,
                    const std::vector<double>& x) const;

        /// Retrieve connate saturation from all tables.
        std::vector<double> connateSat() const;

        /// Retrieve critical saturation for particular result column in all
        /// tables.
        std::vector<double> criticalSat(const ResultColumn& c) const;

        /// Retrieve maximum saturation in all tables.
        std::vector<double> maximumSat() const;

        /// Retrieve unscaled sample points of independent variable in
        /// particular sub-table (saturation region).
        ///
        /// \param[in] t ID of sub-table of interpolant.
        ///
        /// \return Abscissas of tabulated saturation function corresponding
        ///    to particular saturation region.
        const std::vector<double>& saturationPoints(const InTable& t) const;

        /// Retrieve number of internal tables.
        ///
        /// \return Number of internal tables.
        ECLPropTableRawData::SizeType numTables() const
        {
            return this->table_.size();
        }

    private:
        /// Single tabulated 1D interpolant.
        class SingleTable
        {
        public:
            using ElmIt = ECLPropTableRawData::ElementIterator;

            /// Constructor.
            ///
            /// \param[in] xBegin Beginning (initial element) of linar range
            ///    of independent variable values.
            ///
            /// \param[in] xEnd One past the end of linear range of
            ///    independent variable values.
            ///
            /// \param[in] convert Unit conversion support.  Mostly
            ///    applicable to capillary pressure.  Assumed to convert raw
            ///    table data to strict SI unit conventions.
            ///
            /// \param[in,out] colIt Dependent/column range iterators.  On
            ///    input, point to the beginnings of ranges of results
            ///    pertinent to a single table.  On output, each iterator is
            ///    advanced across all rows of the SingleTable (including
            ///    sentinel/invalid nodes) which makes the pointers valid
            ///    for the next table if relevant (and called in a loop).
            SingleTable(ElmIt               xBegin,
                        ElmIt               xEnd,
                        const ConvertUnits& convert,
                        std::vector<ElmIt>& colIt);

            /// Evaluate 1D interpolant in sequence of points.
            ///
            /// \param[in] nCols Number of table columns.
            ///
            /// \param[in] c ID of result column/dependent variable.
            ///
            /// \param[in] x Points at which to evaluate interpolant.
            ///
            /// \return Function values of dependent variable \p c evaluated
            ///    at points \p x.
            std::vector<double>
            interpolate(const ResultColumn&        c,
                        const std::vector<double>& x) const;

            /// Retrieve connate saturation in table.
            double connateSat() const;

            /// Retrieve critical saturation for particular result column in
            /// table.
            double criticalSat(const ResultColumn& c) const;

            /// Retrieve maximum saturation in table.
            double maximumSat() const;

            /// Retrieve unscaled sample points of independent variable.
            const std::vector<double>& saturationPoints() const;

        private:
            /// Extrapolation policy for property evaluator/interpolant.
            using Extrap = ::Opm::Interp1D::PiecewisePolynomial::
                ExtrapolationPolicy::Constant;

            /// Type of fundamental table interpolant.
            using Backend = ::Opm::Interp1D::
                PiecewisePolynomial::Linear<Extrap>;

            Backend interp_;
        };

        /// Number of result/dependent variables (== #table cols - 1).
        ECLPropTableRawData::SizeType nResCols_;

        /// Sequence of individual tables, indexed by *NUM-type vectors.
        std::vector<SingleTable> table_;
    };

} // namespace Opm

#endif // OPM_ECLPROPTABLE_HEADER_INCLUDED
