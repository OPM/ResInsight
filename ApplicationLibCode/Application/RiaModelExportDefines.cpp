/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RiaModelExportDefines.h"

#include "cafAppEnum.h"

namespace caf
{
template <>
void caf::AppEnum<RiaModelExportDefines::GridBoxSelection>::setUp()
{
    addItem( RiaModelExportDefines::VISIBLE_CELLS_BOX, "VISIBLE_CELLS", "Box Containing all Visible Cells" );
    addItem( RiaModelExportDefines::ACTIVE_CELLS_BOX, "ACTIVE_CELLS", "Box Containing all Active Cells" );
    addItem( RiaModelExportDefines::VISIBLE_WELLS_BOX, "VISIBLE_WELLS", "Box Containing all Visible Simulation Wells" );
    addItem( RiaModelExportDefines::FULL_GRID_BOX, "FULL_GRID", "Full Grid" );
    addItem( RiaModelExportDefines::MANUAL_SELECTION, "MANUAL_SELECTION", "User Defined Selection" );

    setDefault( RiaModelExportDefines::VISIBLE_CELLS_BOX );
}

template <>
void caf::AppEnum<RiaModelExportDefines::BoundaryCondition>::setUp()
{
    addItem( RiaModelExportDefines::OPERNUM_OPERATER, "OPERNUM_OPERATER", "OPERNUM + OPERATER" );
    addItem( RiaModelExportDefines::BCCON_BCPROP, "BCCON_BCPROP", "BCCON + BCPROP" );

    setDefault( RiaModelExportDefines::OPERNUM_OPERATER );
}

} // namespace caf
