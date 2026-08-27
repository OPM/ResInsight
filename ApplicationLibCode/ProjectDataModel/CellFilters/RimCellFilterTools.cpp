/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RimCellFilterTools.h"

#include "RigEclipseCaseData.h"
#include "RigGridBase.h"
#include "RigLocalGrid.h"
#include "RigMainGrid.h"

#include "RimCellFilter.h"
#include "RimEclipseCase.h"

//--------------------------------------------------------------------------------------------------
/// Evaluate the filter against the given case for all grids, and return the visibility indexed by
/// reservoir cell index. Property filters are evaluated against the given case's own result values.
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::UByteArray>
    RimCellFilterTools::computeReservoirCellVisibility( RimCellFilter* filter, RimEclipseCase* eclipseCase, size_t timeStepIndex )
{
    if ( !filter || !eclipseCase || !eclipseCase->eclipseCaseData() || !eclipseCase->eclipseCaseData()->mainGrid() ) return nullptr;

    RigEclipseCaseData* caseData = eclipseCase->eclipseCaseData();

    cvf::ref<cvf::UByteArray> reservoirVisibility = new cvf::UByteArray( caseData->mainGrid()->totalCellCount() );
    reservoirVisibility->setAll( false );

    const bool isInclude = ( filter->filterMode() == RimCellFilter::INCLUDE );

    // Grid local masks are kept to allow propagation of parent grid visibility into LGRs
    std::vector<cvf::ref<cvf::UByteArray>> gridMasks( caseData->gridCount() );

    for ( size_t gridIndex = 0; gridIndex < caseData->gridCount(); gridIndex++ )
    {
        RigGridBase* grid = caseData->grid( gridIndex );

        gridMasks[gridIndex]      = new cvf::UByteArray( grid->cellCount() );
        cvf::UByteArray& gridMask = *gridMasks[gridIndex];
        gridMask.setAll( true );

        if ( filter->isRangeFilter() )
        {
            // Range filters evaluate only on their target grid. On other grids an INCLUDE filter
            // contributes no cells, while an EXCLUDE filter removes none.
            const bool isTargetGrid = ( filter->gridIndex() == static_cast<int>( gridIndex ) );
            if ( isTargetGrid )
            {
                filter->applyToCellVisibility( &gridMask, grid, timeStepIndex, eclipseCase );
            }
            else
            {
                gridMask.setAll( !isInclude );
            }
        }
        else
        {
            // Index filters (polygon, user defined) and property filters evaluate on all grids
            filter->applyToCellVisibility( &gridMask, grid, timeStepIndex, eclipseCase );
        }

        // Cells in LGRs follow the visibility of their parent grid cell, as in the filtered geometry
        // of a 3d view. Geometry based filters (range and index, e.g. an INDEX_K polygon) can fail to
        // select the refined cells directly on a fine LGR, so propagate the parent grid visibility.
        // Property filters evaluate each cell against its own result value and must be left untouched.
        const bool isGeometryFilter = filter->isRangeFilter() || filter->isIndexFilter();
        if ( isGeometryFilter && !grid->isMainGrid() )
        {
            auto                   localGrid  = static_cast<const RigLocalGrid*>( grid );
            const cvf::UByteArray& parentMask = *gridMasks[localGrid->parentGrid()->gridIndex()];

            for ( size_t localIdx = 0; localIdx < grid->cellCount(); localIdx++ )
            {
                const size_t parentCellIndex = grid->cell( localIdx ).parentCellIndex();
                if ( isInclude )
                {
                    gridMask[localIdx] = gridMask[localIdx] || parentMask[parentCellIndex];
                }
                else
                {
                    gridMask[localIdx] = gridMask[localIdx] && parentMask[parentCellIndex];
                }
            }
        }

        for ( size_t localIdx = 0; localIdx < grid->cellCount(); localIdx++ )
        {
            reservoirVisibility->set( grid->reservoirCellIndex( localIdx ), gridMask[localIdx] );
        }
    }

    return reservoirVisibility;
}
