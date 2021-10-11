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

#include "RivFemIntersectionGrid.h"

#include "RigFemPart.h"
#include "RigFemPartCollection.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivFemIntersectionGrid::RivFemIntersectionGrid( const RigFemPartCollection* femParts )
    : m_femParts( femParts )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RivFemIntersectionGrid::displayOffset() const
{
    return m_femParts->boundingBox().min();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RivFemIntersectionGrid::boundingBox() const
{
    return m_femParts->boundingBox();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivFemIntersectionGrid::findIntersectingCells( const cvf::BoundingBox& intersectingBB,
                                                    std::vector<size_t>*    intersectedCells ) const
{
    // TODO - create some sort of local list of pair<partId, cellIdx> and use those as "global" indexes
    for ( int i = 0; i < m_femParts->partCount(); i++ )
    {
        m_femParts->part( i )->findIntersectingCells( intersectingBB, intersectedCells );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivFemIntersectionGrid::useCell( size_t cellIndex ) const
{
    // we don't know which part the element we are asked for belongs to, but we only support HEX 8 anyways, so we are
    // good to go.
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivFemIntersectionGrid::cellCornerVertices( size_t cellIndex, cvf::Vec3d cellCorners[8] ) const
{
    const std::vector<cvf::Vec3f>& nodeCoords    = m_femParts->part( 0 )->nodes().coordinates;
    const int*                     cornerIndices = m_femParts->part( 0 )->connectivities( cellIndex );

    cellCorners[0] = cvf::Vec3d( nodeCoords[cornerIndices[0]] );
    cellCorners[1] = cvf::Vec3d( nodeCoords[cornerIndices[1]] );
    cellCorners[2] = cvf::Vec3d( nodeCoords[cornerIndices[2]] );
    cellCorners[3] = cvf::Vec3d( nodeCoords[cornerIndices[3]] );
    cellCorners[4] = cvf::Vec3d( nodeCoords[cornerIndices[4]] );
    cellCorners[5] = cvf::Vec3d( nodeCoords[cornerIndices[5]] );
    cellCorners[6] = cvf::Vec3d( nodeCoords[cornerIndices[6]] );
    cellCorners[7] = cvf::Vec3d( nodeCoords[cornerIndices[7]] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivFemIntersectionGrid::cellCornerIndices( size_t cellIndex, size_t cornerIndices[8] ) const
{
    RigElementType elmType = m_femParts->part( 0 )->elementType( cellIndex );
    if ( !( elmType == HEX8 || elmType == HEX8P ) ) return;
    int elmIdx       = static_cast<int>( cellIndex );
    cornerIndices[0] = m_femParts->part( 0 )->elementNodeResultIdx( elmIdx, 0 );
    cornerIndices[1] = m_femParts->part( 0 )->elementNodeResultIdx( elmIdx, 1 );
    cornerIndices[2] = m_femParts->part( 0 )->elementNodeResultIdx( elmIdx, 2 );
    cornerIndices[3] = m_femParts->part( 0 )->elementNodeResultIdx( elmIdx, 3 );
    cornerIndices[4] = m_femParts->part( 0 )->elementNodeResultIdx( elmIdx, 4 );
    cornerIndices[5] = m_femParts->part( 0 )->elementNodeResultIdx( elmIdx, 5 );
    cornerIndices[6] = m_femParts->part( 0 )->elementNodeResultIdx( elmIdx, 6 );
    cornerIndices[7] = m_femParts->part( 0 )->elementNodeResultIdx( elmIdx, 7 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigFault* RivFemIntersectionGrid::findFaultFromCellIndexAndCellFace( size_t reservoirCellIndex,
                                                                           cvf::StructGridInterface::FaceType face ) const
{
    return nullptr;
}
