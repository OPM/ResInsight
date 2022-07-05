/*
  Copyright (C) 2020 SINTEF Digital

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

#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include "AquiferHelpers.hpp"

namespace Opm::AquiferHelpers {
    bool cellInsideReservoirAndActive(const EclipseGrid& grid, const int i, const int j, const int k,
                                      const std::vector<int>& actnum)
    {
        if ( i < 0 || j < 0 || k < 0
             || size_t(i) > grid.getNX() - 1
             || size_t(j) > grid.getNY() - 1
             || size_t(k) > grid.getNZ() - 1 )
        {
            return false;
        }

        const size_t globalIndex = grid.getGlobalIndex(i,j,k);
        return actnum[globalIndex];
    }

    bool neighborCellInsideReservoirAndActive(const EclipseGrid& grid, const int i, const int j, const int k,
                                              const Opm::FaceDir::DirEnum faceDir, const std::vector<int>& actnum)
    {
        switch(faceDir) {
            case FaceDir::XMinus:
                return cellInsideReservoirAndActive(grid, i - 1, j, k, actnum);
            case FaceDir::XPlus:
                return cellInsideReservoirAndActive(grid, i + 1, j, k, actnum);
            case FaceDir::YMinus:
                return cellInsideReservoirAndActive(grid, i, j - 1, k, actnum);
            case FaceDir::YPlus:
                return cellInsideReservoirAndActive(grid, i, j + 1, k, actnum);
            case FaceDir::ZMinus:
                return cellInsideReservoirAndActive(grid, i, j, k - 1, actnum);
            case FaceDir::ZPlus:
                return cellInsideReservoirAndActive(grid, i, j, k + 1, actnum);
            default:
                throw std::runtime_error("Unknown FaceDir enum " + std::to_string(faceDir));
        }
    }
}
