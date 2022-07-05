/*
  Copyright (c) 2021 Equinor ASA

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

#include <opm/output/eclipse/ActiveIndexByColumns.hpp>

#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <functional>
#include <numeric>
#include <vector>

namespace {
    std::size_t columnarGlobalIdx(const std::array<int, 3>& dims,
                                  const std::array<int, 3>& ijk)
    {
        // Linear index assuming C-like loop order
        //
        //     for (i = 0 .. Nx - 1)
        //         for (j = 0 .. Ny - 1)
        //             for (k = 0 .. Nz - 1)
        //
        // as opposed to the usual Fortran-like loop order ("natural ordering")
        //
        //     for (k = 0 .. Nz - 1)
        //         for (j = 0 .. Ny - 1)
        //             for (i = 0 .. Nx - 1)
        //
        return ijk[2] + dims[2]*(ijk[1] + dims[1]*ijk[0]);
    }

    std::vector<int>
    buildMappingTables(const std::size_t                                           numActive,
                       const std::array<int, 3>&                                   cartDims,
                       const std::function<std::array<int, 3>(const std::size_t)>& getIJK)
    {
        auto natural2columnar = std::vector<int>(numActive, 0);

        auto activeCells = std::vector<std::size_t>(numActive, std::size_t{0});
        std::iota(activeCells.begin(), activeCells.end(), std::size_t{0});

        std::sort(activeCells.begin(), activeCells.end(),
            [&cartDims, &getIJK](const std::size_t cell1, const std::size_t cell2) -> bool
        {
            return columnarGlobalIdx(cartDims, getIJK(cell1))
                <  columnarGlobalIdx(cartDims, getIJK(cell2));
        });

        auto columnarActiveID = 0;
        for (const auto& naturalActiveID : activeCells) {
            natural2columnar[naturalActiveID] = columnarActiveID++;
        }

        return natural2columnar;
    }
}

bool Opm::ActiveIndexByColumns::operator==(const ActiveIndexByColumns& rhs) const
{
    return this->natural2columnar_ == rhs.natural2columnar_;
}

Opm::ActiveIndexByColumns::
ActiveIndexByColumns(const std::size_t                                           numActive,
                     const std::array<int, 3>&                                   cartDims,
                     const std::function<std::array<int, 3>(const std::size_t)>& getIJK)
    : natural2columnar_{ buildMappingTables(numActive, cartDims, getIJK) }
{}

Opm::ActiveIndexByColumns
Opm::buildColumnarActiveIndexMappingTables(const EclipseGrid& grid)
{
    return ActiveIndexByColumns { grid.getNumActive(), grid.getNXYZ(),
        [&grid](const std::size_t activeCell)
    {
        return grid.getIJK(grid.getGlobalIndex(activeCell));
    }};
}
