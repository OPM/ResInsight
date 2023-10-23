/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023  Equinor ASA
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

#include "RimFaultReactivationDataAccessor.h"

// #include "RiaDefines.h"
// #include "RiaPorosityModel.h"

// #include "RigCaseCellResultsData.h"
// #include "RigEclipseCaseData.h"
// #include "RigEclipseResultAddress.h"
// #include "RigFault.h"
#include "RigMainGrid.h"
// #include "RigResultAccessorFactory.h"

// #include "RimEclipseCase.h"
// #include "RimFaultReactivationEnums.h"

#include "cafAssert.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationDataAccessor::RimFaultReactivationDataAccessor()
{
    m_timeStep = -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationDataAccessor::~RimFaultReactivationDataAccessor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationDataAccessor::setTimeStep( size_t timeStep )
{
    m_timeStep = timeStep;
    updateResultAccessor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationDataAccessor::useCellIndexAdjustment( const std::map<size_t, size_t>& adjustments )
{
    m_cellIndexAdjustment = adjustments;
}

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
size_t RimFaultReactivationDataAccessor::findAdjustedCellIndex( const cvf::Vec3d&               position,
                                                                const RigMainGrid*              grid,
                                                                const std::map<size_t, size_t>& cellIndexAdjustmentMap )
{
    CAF_ASSERT( grid != nullptr );

    size_t cellIdx = grid->findReservoirCellIndexFromPoint( position );

    // adjust cell index if present in the map
    if ( auto search = cellIndexAdjustmentMap.find( cellIdx ); search != cellIndexAdjustmentMap.end() )
    {
        cellIdx = search->second;
    }

    return cellIdx;
}
