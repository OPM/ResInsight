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
        DataVector data;

        /// Number of rows allocated in the result set for each individual
        /// table.  Typically corresponds to setting in one of the *DIMS
        /// keywords.  Should normally be at least two.
        SizeType numRows;

        /// Number of columns in this table.  Varies by keyword/table.
        SizeType numCols;

        /// Number of tables of this type.  Must match the corresponding
        /// region keyword.
        SizeType numTables;
    };

    /// Collection of 1D interpolants from tabulated functions (e.g., the
    /// saturation functions).
    class SatFuncInterpolant
    {
    public:
        /// Constructor.
        ///
        /// \param[in] raw Raw table data for this collection.
        explicit SatFuncInterpolant(const ECLPropTableRawData& raw);

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
            /// \param[in,out] colIt Dependent/column range iterators.  On
            ///    input, point to the beginnings of ranges of results
            ///    pertinent to a single table.  On output, each iterator is
            ///    advanced across all rows of the SingleTable (including
            ///    sentinel/invalid nodes) which makes the pointers valid
            ///    for the next table if relevant (and called in a loop).
            SingleTable(ElmIt               xBegin,
                        ElmIt               xEnd,
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
            interpolate(const ECLPropTableRawData::SizeType nCols,
                        const ResultColumn&                 c,
                        const std::vector<double>&          x) const;

            /// Retrieve connate saturation in table.
            double connateSat() const;

            /// Retrieve critical saturation for particular result column in
            /// table.
            double criticalSat(const ECLPropTableRawData::SizeType nCols,
                               const ResultColumn&                 c) const;

            /// Retrieve maximum saturation in table.
            double maximumSat() const;

        private:
            /// Independent variable.
            std::vector<double> x_;

            /// Dependent variable (or variables).  Row major (i.e., C)
            /// ordering.  Number of elements: x_.size() * host.nCols_.
            std::vector<double> y_;

            /// Value of dependent variable at position (row,c).
            double y(const ECLPropTableRawData::SizeType nCols,
                     const ECLPropTableRawData::SizeType row,
                     const ResultColumn&                 c) const;
        };

        /// Number of result/dependent variables (== #table cols - 1).
        ECLPropTableRawData::SizeType nResCols_;

        /// Sequence of individual tables, indexed by *NUM-type vectors.
        std::vector<SingleTable> table_;
    };

} // namespace Opm

#endif // OPM_ECLPROPTABLE_HEADER_INCLUDED
