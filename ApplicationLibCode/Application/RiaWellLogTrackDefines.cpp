/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "RiaWellLogTrackDefines.h"

namespace caf
{
template <>
void AppEnum<RiaDefines::WellLogTrackTrajectoryType>::setUp()
{
    addItem( RiaDefines::WellLogTrackTrajectoryType::WELL_PATH, "WELL_PATH", "Well Path" );
    addItem( RiaDefines::WellLogTrackTrajectoryType::SIMULATION_WELL, "SIMULATION_WELL", "Simulation Well" );
    setDefault( RiaDefines::WellLogTrackTrajectoryType::WELL_PATH );
}

template <>
void AppEnum<RiaDefines::WellLogTrackFormationSource>::setUp()
{
    addItem( RiaDefines::WellLogTrackFormationSource::CASE, "CASE", "Case" );
    addItem( RiaDefines::WellLogTrackFormationSource::WELL_PICK_FILTER, "WELL_PICK_FILTER", "Well Picks for Well Path" );
    setDefault( RiaDefines::WellLogTrackFormationSource::CASE );
}

template <>
void AppEnum<RiaDefines::WellLogTrackFormationLevel>::setUp()
{
    addItem( RiaDefines::WellLogTrackFormationLevel::NONE, "NONE", "None" );
    addItem( RiaDefines::WellLogTrackFormationLevel::ALL, "ALL", "All" );
    addItem( RiaDefines::WellLogTrackFormationLevel::GROUP, "GROUP", "Formation Group" );
    addItem( RiaDefines::WellLogTrackFormationLevel::LEVEL0, "LEVEL0", "Formation" );
    addItem( RiaDefines::WellLogTrackFormationLevel::LEVEL1, "LEVEL1", "Formation 1" );
    addItem( RiaDefines::WellLogTrackFormationLevel::LEVEL2, "LEVEL2", "Formation 2" );
    addItem( RiaDefines::WellLogTrackFormationLevel::LEVEL3, "LEVEL3", "Formation 3" );
    addItem( RiaDefines::WellLogTrackFormationLevel::LEVEL4, "LEVEL4", "Formation 4" );
    addItem( RiaDefines::WellLogTrackFormationLevel::LEVEL5, "LEVEL5", "Formation 5" );
    addItem( RiaDefines::WellLogTrackFormationLevel::LEVEL6, "LEVEL6", "Formation 6" );
    addItem( RiaDefines::WellLogTrackFormationLevel::LEVEL7, "LEVEL7", "Formation 7" );
    addItem( RiaDefines::WellLogTrackFormationLevel::LEVEL8, "LEVEL8", "Formation 8" );
    addItem( RiaDefines::WellLogTrackFormationLevel::LEVEL9, "LEVEL9", "Formation 9" );
    addItem( RiaDefines::WellLogTrackFormationLevel::LEVEL10, "LEVEL10", "Formation 10" );
    setDefault( RiaDefines::WellLogTrackFormationLevel::ALL );
}

} // namespace caf
