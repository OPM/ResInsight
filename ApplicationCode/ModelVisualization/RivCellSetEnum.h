/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

enum RivCellSetEnum
{
    OVERRIDDEN_CELL_VISIBILITY, ////< Use the total visibility from a different case directly
    ALL_CELLS,
    ACTIVE, ///< All Active cells without ALL_WELL_CELLS
    ALL_WELL_CELLS, ///< All cells ever having a connection to a well (Might be inactive cells as well. Wellhead cells
                    ///< typically)
    VISIBLE_WELL_CELLS, ///< ALL_WELL_CELLS && visible well cells (including Fence <-- is this correct? MSJ/JJS )
    VISIBLE_WELL_FENCE_CELLS, ///< (! ALL_WELL_CELLS) && visible well cells including Fence
    INACTIVE, ///< All inactive cells, but invalid cells might or might not be included
    RANGE_FILTERED, ///< ACTIVE Filtered by the set of range filters
    RANGE_FILTERED_INACTIVE, ///< INACTIVE Filtered by the set of range filters
    RANGE_FILTERED_WELL_CELLS, ///< ALL_WELL_CELLS Filtered by the set of range filters
    VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER, ///< VISIBLE_WELL_CELLS && !RANGE_FILTERED_WELL_CELLS
    VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER, ///< VISIBLE_WELL_FENCE_CELLS && !RANGE_FILTERED
    PROPERTY_FILTERED, ///< (RANGE_FILTERED || VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER) && !ExcludedByPropFilter
                       ///< && IncludedByPropFilter
    PROPERTY_FILTERED_WELL_CELLS ///< (!(hasActiveRangeFilters || visibleWellCells) && (*ALL_WELL_CELLS)) ||
                                 ///< RANGE_FILTERED_WELL_CELLS || VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER
};
