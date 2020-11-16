/*
  Copyright 2019 Equinor.
  Copyright 2017 Statoil ASA.

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

#ifndef LINEARISED_OUTPUT_TABLE_HPP_INCLUDED
#define LINEARISED_OUTPUT_TABLE_HPP_INCLUDED

#include <cstddef>
#include <vector>

namespace Opm {

    /// Manage tables of column data, possibly with sub-tables, all with
    /// equal number of rows (i.e., with padding), and possibly with
    /// multiple tables pertaining to multiple subsets (i.e., cell regions).
    ///
    /// Mainly intended for use with the output facility for tabular data.
    class LinearisedOutputTable
    {
    public:
        /// Constructor.
        ///
        /// Padded table entries set to +1.0e+20.
        ///
        /// \param[in] numTables Number of tables managed by internal
        ///    buffer.  Typically corresponds to maximum value of a region
        ///    ID vector such as SATNUM, IMBNUM, or PVTNUM.
        ///
        /// \param[in] numPrimary Number of primary look-up keys for the
        ///    tabular data managed by the internal buffer.  Mostly relevant
        ///    (i.e., greater than one) for miscible oil ("PVTO") or
        ///    miscible gas ("PVTG") tables and typically corresponds to the
        ///    number of Rs/Rv entries from the TABDIMS keyword.
        ///
        /// \param[in] numRows Number of rows in each sub-table
        ///    corresponding to a single primary look-up key.  Typically the
        ///    number of nodes (e.g., NSSFUN or NPPVT) of the corresponding
        ///    input keyword.
        ///
        /// \param[in] numCols Number of columns in each sub-table (and main
        ///    table).  Typically 5.
        LinearisedOutputTable(const std::size_t numTables,
                              const std::size_t numPrimary,
                              const std::size_t numRows,
                              const std::size_t numCols);

        /// Constructor.
        ///
        /// \param[in] numTables Number of tables managed by internal
        ///    buffer.  Typically corresponds to maximum value of a region
        ///    ID vector such as SATNUM, IMBNUM, or PVTNUM.
        ///
        /// \param[in] numPrimary Number of primary look-up keys for the
        ///    tabular data managed by the internal buffer.  Mostly relevant
        ///    (i.e., greater than one) for miscible oil ("PVTO") or
        ///    miscible gas ("PVTG") tables and typically corresponds to the
        ///    number of Rs/Rv entries from the TABDIMS keyword.
        ///
        /// \param[in] numRows Number of rows in each sub-table
        ///    corresponding to a single primary look-up key.  Typically the
        ///    number of nodes (e.g., NSSFUN or NPPVT) of the corresponding
        ///    input keyword.
        ///
        /// \param[in] numCols Number of columns in each sub-table (and main
        ///    table).  Typically 5.
        ///
        /// \param[in] fillValue Data element value for padded table entries.
        LinearisedOutputTable(const std::size_t numTables,
                              const std::size_t numPrimary,
                              const std::size_t numRows,
                              const std::size_t numCols,
                              const double      fillValue);

        /// Retrieve iterator to start of \c numRows (contiguous) column
        /// elements of a particular sub-table of a particular main table.
        ///
        /// \param[in] tableID Numeric ID of main table for which to
        ///    retrieve a column.  Must be strictly less than \c numTables
        ///    constructor argument.
        ///
        /// \param[in] primID Numeric ID of primary look-up key (sub-table)
        ///    for which to retrieve a column.  Must be strictly less than
        ///    \c numPrimary constructor argument.
        ///
        /// \param[in] colID Numeric ID of column to be retrieved from
        ///    particular sub-table of particular main table.  Must be
        ///    strictly less than \c numCols constructor argument.
        ///
        /// \return Iterator to start of contiguous column elements.
        std::vector<double>::iterator
        column(const std::size_t tableID,
               const std::size_t primID,
               const std::size_t colID);

        /// Read-only access to internal data buffer.
        ///
        /// Mostly to support outputting all table data to external storage.
        const std::vector<double>& getData() const;

        /// Destructive access to internal data buffer.
        ///
        /// Mostly to support outputting all table data to external storage.
        ///
        /// \return \code std::move() \endcode of the internal data buffer.
        std::vector<double> getDataDestructively();

    private:
        /// Internal buffer for tabular data.
        std::vector<double> data;

        /// Number of tables managed by \c data.
        std::size_t numTables;

        /// Number of primary look-up keys/sub-tables in \c data_.
        std::size_t numPrimary;

        /// Number of rows per sub-table in \c data_.
        std::size_t numRows;
    };

    /// Apply piecewise linear differentiation (i.e., compute slopes) on a
    /// set of dependent variables in a linearised output table.
    ///
    namespace DifferentiateOutputTable {
        /// Columnar data differentantiation table request.
        ///
        /// Refers to the sub-table with a specific primary ID within a
        /// specific table of a \c LinearisedOutputTable.
        struct Descriptor {
            /// Table ID--usually corresponds to the region ID of a
            ///   tabulated function pertaining to a specific region of a
            ///   simulation model.
            std::size_t tableID{ 0 };

            /// Primary ID--nontrivial (\c != 0) only for miscible PVT
            ///   tables for oil or gas in which case this entry refers to a
            ///   particular dissolved gas-oil ratio (Rs) or gas pressure
            ///   (Pg) node.
            std::size_t primID{ 0 };

            /// Number of active rows in this subtable.
            std::size_t numActRows{ 0 };
        };

        /// Apply piecewise linear differentiation (i.e., compute slopes) on
        /// a set of dependent variables in a linearised output table.
        ///
        /// Assumes that the independent variable is stored in the first
        /// column (column ID zero).
        ///
        /// \param[in] numDependent Number of dependent variables.  Usually
        ///    one or two.  Dependent variables are assumed to be stored in
        ///    columns one through \p numDependent.
        ///
        /// \param[in] desc Columnar data differentantiation table request.
        ///    Must refer to a particular sub-table of the linearised output
        ///    table.
        ///
        /// \param[in,out] table Linearised output table.  On input, column
        ///    zero contains the independent variable in each of \code
        ///    numActRows.size() \endcode sub-tables and columns one through
        ///    \p numDependent contain the dependent variables.  On output,
        ///    columns \code numDependent + 1 \endcode through \code 2 *
        ///    numDependent \endcode contain the slopes of the dependent
        ///    variables.
        ///
        ///    In partcular, column \code numDependent + j \endcode for
        ///    \code j = 1..numDependent \endcode contains the derivatives
        ///    of column \c j with respect to column zero.  We define the
        ///    slopes as
        ///    \code
        ///       s(i,j) = (c(i + 1, j) - c(i,j)) / (c(i + 1, 0) - c(i,0))
        ///    \endcode
        ///    for all \code i = 0 .. numActRows[k] - 2 \endcode (in table
        ///    \p k).
        void calcSlopes(const std::size_t      numDependent,
                        const Descriptor&      desc,
                        LinearisedOutputTable& table);
    } // DifferentiateOutputTable
} // Opm

#endif // LINEARISED_OUTPUT_TABLE_HPP_INCLUDED
