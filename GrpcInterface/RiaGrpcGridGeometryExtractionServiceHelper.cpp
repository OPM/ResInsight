/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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
//////////////////////////////////////////////////////////////////////////////////

#include "RiaGrpcGridGeometryExtractionServiceHelper.h"

#include "RigMainGrid.h"
#include "RigNNCData.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigGridCellFaultFaceVisibilityFilter::isFaceVisible( size_t                             i,
                                                          size_t                             j,
                                                          size_t                             k,
                                                          cvf::StructGridInterface::FaceType face,
                                                          const cvf::UByteArray*             cellVisibility ) const
{
    CVF_TIGHT_ASSERT( m_grid );

    size_t          cellIndex           = m_grid->cellIndexFromIJK( i, j, k );
    size_t          nativeResvCellIndex = m_grid->reservoirCellIndex( cellIndex );
    const RigFault* fault = m_grid->mainGrid()->findFaultFromCellIndexAndCellFace( nativeResvCellIndex, face );
    if ( fault )
    {
        // TODO: REMOVE EARLY RETURN
        return true; // TMP: Until we have a better way to determine if a fault face should be visible

        // If no nnc data is generated, include fault face based on the flag
        auto* nncData = m_grid->mainGrid()->nncData();
        if ( nncData == nullptr || nncData->allConnections().empty() )
        {
            return true;
        }

        // Do not show fault if inside the grid
        const auto& faultConnectionIndices = fault->connectionIndices();
        for ( const auto& connectionIndex : faultConnectionIndices )
        {
            if ( connectionIndex >= nncData->allConnections().size() )
            {
                continue;
            }

            const RigConnection& connection = nncData->allConnections()[connectionIndex];

            size_t oppositeCellIndex = std::numeric_limits<size_t>::max();
            if ( connection.c1GlobIdx() == cellIndex )
            {
                oppositeCellIndex = connection.c2GlobIdx();
            }
            else if ( connection.c2GlobIdx() == cellIndex )
            {
                oppositeCellIndex = connection.c1GlobIdx();
            }

            if ( oppositeCellIndex >= cellVisibility->size() )
            {
                continue;
            }

            // TODO: This gives false for all fault faces, is cellVisibility incorrect?
            // - Only want to include faults on the boundary of the surface, not the ones inside the grid
            // - Now it excludes all, as all opposite cells are visible
            // - Seems not to work if fault is on the boundary of the grid?
            auto isOppositeCellVisible = ( *cellVisibility )[oppositeCellIndex] != 0;
            if ( !isOppositeCellVisible )
            {
                return true;
            }
        }

        // All opposite cells are visible, do not show the fault face
        return false;
    }
    return false;
}
