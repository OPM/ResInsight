/*
  Copyright 2021 Equinor ASA.

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
#ifndef SCHEDULE_GRID
#define SCHEDULE_GRID

#include <opm/input/eclipse/Schedule/CompletedCells.hpp>

namespace Opm {

class EclipseGrid;
class FieldPropsManager;

class ScheduleGrid {
public:
    ScheduleGrid(const EclipseGrid& ecl_grid, const FieldPropsManager& fpm, CompletedCells& completed_cells);
    explicit ScheduleGrid(CompletedCells& completed_cells);

    const CompletedCells::Cell& get_cell(std::size_t i, std::size_t j, std::size_t k) const;

private:
    const EclipseGrid* grid{nullptr};
    const FieldPropsManager* fp{nullptr};
    CompletedCells& cells;
};



}
#endif

