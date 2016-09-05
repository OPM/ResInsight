/*
  Copyright 2016 SINTEF ICT, Applied Mathematics.
  Copyright 2016 Statoil ASA.

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

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <opm/flowdiagnostics/CellSetValues.hpp>

#include <cassert>
#include <utility>

namespace Opm {
namespace FlowDiagnostics {

CellSetValues::CellSetValues(const SizeType initialCapacity)
{
    assoc_.reserve(initialCapacity);
}

void
CellSetValues::addCellValue(const int    cellIndex,
                                 const double cellValue)
{
    assoc_.push_back(Association{cellIndex, cellValue});
}

CellSetValues::SizeType
CellSetValues::cellValueCount() const
{
    return assoc_.size();
}

CellSetValues::CellValue
CellSetValues::cellValue(const SizeType cellValueIndex) const
{
    const auto& a = assoc_[cellValueIndex];

    return { a.index, a.value };
}

} // namespace FlowDiagnostics
} // namespace Opm
