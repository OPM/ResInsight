/*
  Copyright 2016 Statoil ASA.
  Copyright 2016 SINTEF ICT, Applied Mathematics.

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

#ifndef OPM_CELLSETVALUES_HEADER_INCLUDED
#define OPM_CELLSETVALUES_HEADER_INCLUDED

#include <utility>
#include <vector>

namespace Opm {
namespace FlowDiagnostics {

    class CellSetValues
    {
    public:
        using SizeType  = std::vector<int>::size_type;
        using CellValue = std::pair<int, double>;

        /// Constructor.
        ///
        /// @param[in] initialCapacity Number of elements that can be stored
        ///            in set without reallocation.
        explicit CellSetValues(const SizeType initialCapacity = 0);

        /// Associate value with specific cell, represented by its index.
        ///
        /// @param[in] cellIndex Index of specific cell.
        ///
        /// @param[in] cellValue Value associated with cell @p cellIndex.
        void addCellValue(const int    cellIndex,
                          const double cellValue);

        /// Retrieve number of elements stored in set.
        SizeType cellValueCount() const;

        /// Retrieve value association for single set element.
        ///
        /// @param[in] cellValueIndex Linear ID of single cell->value
        ///            association.  Must be in the range @code [0,
        ///            cellValueCount()) @endcode.
        ///
        /// @returns Single association between cell index and numerical
        /// value.
        CellValue cellValue(const SizeType cellValueIndex) const;

    private:
        struct Association
        {
            int    index;
            double value;
        };

        std::vector<Association> assoc_;
    };

} // namespace FlowDiagnostics
} // namespace Opm

#endif // OPM_CELLSETVALUES_HEADER_INCLUDED
