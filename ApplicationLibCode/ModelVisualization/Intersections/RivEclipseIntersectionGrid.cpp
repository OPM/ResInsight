/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 - Equinor ASA
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

#include "RivEclipseIntersectionGrid.h"

#include "RigActiveCellInfo.h"
#include "RigFemPart.h"
#include "RigMainGrid.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivEclipseIntersectionGrid::RivEclipseIntersectionGrid( const RigMainGrid* mainGrid, const RigActiveCellInfo* activeCellInfo, bool showInactiveCells )
    : m_mainGrid( mainGrid )
    , m_activeCellInfo( activeCellInfo )
    , m_showInactiveCells( showInactiveCells )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RivEclipseIntersectionGrid::displayOffset() const
{
    return m_mainGrid->displayModelOffset();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RivEclipseIntersectionGrid::boundingBox() const
{
    return m_mainGrid->boundingBox();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RivEclipseIntersectionGrid::findIntersectingCells( const cvf::BoundingBox& intersectingBB ) const
{
    return m_mainGrid->findIntersectingCells( intersectingBB );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivEclipseIntersectionGrid::useCell( size_t cellIndex ) const
{
    size_t i, j, k;
    m_mainGrid->ijkFromCellIndexUnguarded( cellIndex, &i, &j, &k );

    if ( m_intervalTool.isNumberIncluded( k ) )
    {
        const RigCell& cell = m_mainGrid->globalCellArray()[cellIndex];
        if ( m_showInactiveCells )
        {
            return !cell.isInvalid() && ( cell.subGrid() == nullptr );
        }
        if ( m_activeCellInfo.p() != nullptr )
        {
            return m_activeCellInfo->isActive( cellIndex ) && ( cell.subGrid() == nullptr );
        }
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivEclipseIntersectionGrid::cellCornerVertices( size_t cellIndex, cvf::Vec3d cellCorners[8] ) const
{
    m_mainGrid->cellCornerVertices( cellIndex, cellCorners );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivEclipseIntersectionGrid::cellCornerIndices( size_t cellIndex, size_t cornerIndices[8] ) const
{
    const std::array<size_t, 8>& cornerIndicesSource = m_mainGrid->globalCellArray()[cellIndex].cornerIndices();

    for ( size_t i = 0; i < 8; i++ )
    {
        cornerIndices[i] = cornerIndicesSource[i];
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigFault* RivEclipseIntersectionGrid::findFaultFromCellIndexAndCellFace( size_t                             reservoirCellIndex,
                                                                               cvf::StructGridInterface::FaceType face ) const
{
    return m_mainGrid->findFaultFromCellIndexAndCellFace( reservoirCellIndex, face );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivEclipseIntersectionGrid::setKIntervalFilter( bool enabled, std::string kIntervalStr )
{
    m_intervalTool.setInterval( enabled, kIntervalStr );
}
