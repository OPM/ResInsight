/*
  Copyright 2019 Equinor ASA

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

#include <opm/common/utility/ActiveGridCells.hpp>

#include <iterator>
#include <algorithm>

namespace Opm
{
ActiveGridCells::ActiveGridCells(std::array<int, 3> xyz,
                                 const int* globalCell, std::size_t nc)
    : ActiveGridCells(xyz[0], xyz[1], xyz[2], globalCell, nc)
{}

ActiveGridCells::ActiveGridCells(std::size_t nx, std::size_t ny, std::size_t nz,
                                 const int* globalCell, std::size_t nc)
    : GridDims(nx, ny, nz), localCell_(nx*ny*nz, -1)
{
    for (auto cell = globalCell, cellEnd = globalCell + nc; cell != cellEnd; ++cell)
    {
        localCell_[*cell] = cell-globalCell;
    }
}

bool ActiveGridCells::cellActive(std::size_t i, std::size_t j, std::size_t k) const
{
    return cellActive(this->getGlobalIndex(i,j,k));
}

bool ActiveGridCells::cellActive(std::size_t cartesianIndex) const
{
    return localCell_[cartesianIndex]>=0;
}

int ActiveGridCells::localCell(std::size_t cartesianIndex) const
{
    return localCell_[cartesianIndex];
}

int ActiveGridCells::localCell(std::size_t i, std::size_t j, std::size_t k) const
{
    return localCell(this->getGlobalIndex(i,j,k));
}

std::vector<int> ActiveGridCells::actNum() const
{
    std::vector<int> actnum;
    actnum.reserve(localCell_.size());
    std::transform(localCell_.begin(), localCell_.end(),
                   std::back_inserter(actnum), [](int i){ return i>=0;});
    return actnum;
}
}
