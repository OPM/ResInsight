/*
  Copyright 2021 Statoil ASA.

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

#include <opm/input/eclipse/EclipseState/Grid/SparseScheduleGrid.hpp>

#include <stdexcept>

Opm::SparseScheduleGrid::Cell Opm::SparseScheduleGrid::loadCell(const Opm::ScheduleGrid& source, const Opm::ScheduleGrid::CellKey& key) {
    std::size_t
        i { key[0] } ,
        j { key[1] } ,
        k { key[2] } ;

    std::optional<std::size_t> activeIndex {};

    if (source.isCellActive(i, j, k)) {
        activeIndex = source.getActiveIndex(i, j, k);
    }

    Cell loadedCell {
        i, j, k,
        source.getGlobalIndex(i, j, k),
        activeIndex,

        source.getCellDepth(i, j, k),
        source.getCellDimensions(i, j, k),
    };

    return loadedCell;
}

Opm::SparseScheduleGrid::CellMap Opm::SparseScheduleGrid::loadCells(const Opm::ScheduleGrid& source, const std::set<Opm::ScheduleGrid::CellKey>& loadKeys) {
        Opm::SparseScheduleGrid::CellMap loadedCells { } ;

        for (const auto &key : loadKeys) {
            loadedCells[key] = loadCell(source, key);
        }

        return loadedCells;
    }

Opm::SparseScheduleGrid::SparseScheduleGrid(const Opm::ScheduleGrid& source, const std::set<Opm::ScheduleGrid::CellKey>& loadKeys)
    : loadedCells { loadCells(source, loadKeys) }
{
}

const Opm::SparseScheduleGrid::Cell& Opm::SparseScheduleGrid::getCell(std::size_t i, std::size_t j, std::size_t k) const {
    const auto iter { loadedCells.find({i, j, k}) } ;

    if (iter == loadedCells.end()) {
        throw std::logic_error("BUG: SparseScheduleGrid::getCell called on a sparse grid missing the cell");
    }

    return iter->second;
}

std::size_t Opm::SparseScheduleGrid::getActiveIndex(std::size_t i, std::size_t j, std::size_t k) const {
    const auto& cell { getCell(i, j, k) } ;

    return cell.activeIndex.value();
}

std::size_t Opm::SparseScheduleGrid::getGlobalIndex(std::size_t i, std::size_t j, std::size_t k) const {
    const auto& cell { getCell(i, j, k) } ;

    return cell.globalIndex;
}

bool Opm::SparseScheduleGrid::isCellActive(std::size_t i, std::size_t j, std::size_t k) const {
    const auto& cell { getCell(i, j, k) } ;

    return bool(cell.activeIndex);
}

double Opm::SparseScheduleGrid::getCellDepth(std::size_t i, std::size_t j, std::size_t k) const {
    const auto& cell { getCell(i, j, k) } ;

    return cell.depth;
}

std::array<double, 3> Opm::SparseScheduleGrid::getCellDimensions(std::size_t i, std::size_t j, std::size_t k) const {
    const auto& cell { getCell(i, j, k) } ;

    return cell.dimensions;
}
