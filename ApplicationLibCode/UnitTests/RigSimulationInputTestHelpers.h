/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025- Equinor ASA
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

#include "RigNoRefinement.h"
#include "RigSimulationInputSettings.h"
#include "RigUniformRefinement.h"

#include "cafVecIjk.h"
#include "cvfVector3.h"

#include <memory>

namespace RigSimulationInputTestHelpers
{
//--------------------------------------------------------------------------------------------------
/// Uniform refinement for the sector given by min/max
//--------------------------------------------------------------------------------------------------
inline RigUniformRefinement makeRef( const caf::VecIjk0& sectorMin, const caf::VecIjk0& sectorMax, const cvf::Vec3st& refCounts )
{
    cvf::Vec3st sectorSize( sectorMax.i() - sectorMin.i() + 1, sectorMax.j() - sectorMin.j() + 1, sectorMax.k() - sectorMin.k() + 1 );
    return RigUniformRefinement( refCounts, sectorSize );
}

//--------------------------------------------------------------------------------------------------
/// Set refinement on settings, computing sector size from min/max.
/// If refCounts is (1,1,1), creates RigNoRefinement; otherwise RigUniformRefinement.
//--------------------------------------------------------------------------------------------------
inline void setSettingsRefinement( RigSimulationInputSettings& settings, const cvf::Vec3st& refCounts )
{
    cvf::Vec3st sectorSize( settings.max().i() - settings.min().i() + 1,
                            settings.max().j() - settings.min().j() + 1,
                            settings.max().k() - settings.min().k() + 1 );

    if ( refCounts.x() == 1 && refCounts.y() == 1 && refCounts.z() == 1 )
    {
        settings.setRefinement( std::make_unique<RigNoRefinement>( sectorSize ) );
    }
    else
    {
        settings.setRefinement( std::make_unique<RigUniformRefinement>( refCounts, sectorSize ) );
    }
}
} // namespace RigSimulationInputTestHelpers
