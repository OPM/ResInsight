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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivFemIntersectionGrid::RivFemIntersectionGrid( const RigFemPart* femPart )
    : m_femPart( femPart )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RivFemIntersectionGrid::displayOffset() const
{
    return m_femPart->boundingBox().min();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RivFemIntersectionGrid::boundingBox() const
{
    return m_femPart->boundingBox();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivFemIntersectionGrid::findIntersectingCells( const cvf::BoundingBox& intersectingBB,
                                                    std::vector<size_t>*    intersectedCells ) const
{
    m_femPart->findIntersectingCells( intersectingBB, intersectedCells );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivFemIntersectionGrid::useCell( size_t cellIndex ) const
{
    RigElementType elmType = m_femPart->elementType( cellIndex );

    if ( !( elmType == HEX8 || elmType == HEX8P ) ) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivFemIntersectionGrid::cellCornerVertices( size_t cellIndex, cvf::Vec3d cellCorners[8] ) const
{
    RigElementType elmType = m_femPart->elementType( cellIndex );
    if ( !( elmType == HEX8 || elmType == HEX8P ) ) return;

    const std::vector<cvf::Vec3f>& nodeCoords    = m_femPart->nodes().coordinates;
    const int*                     cornerIndices = m_femPart->connectivities( cellIndex );

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
    RigElementType elmType = m_femPart->elementType( cellIndex );
    if ( !( elmType == HEX8 || elmType == HEX8P ) ) return;
    int elmIdx       = static_cast<int>( cellIndex );
    cornerIndices[0] = m_femPart->elementNodeResultIdx( elmIdx, 0 );
    cornerIndices[1] = m_femPart->elementNodeResultIdx( elmIdx, 1 );
    cornerIndices[2] = m_femPart->elementNodeResultIdx( elmIdx, 2 );
    cornerIndices[3] = m_femPart->elementNodeResultIdx( elmIdx, 3 );
    cornerIndices[4] = m_femPart->elementNodeResultIdx( elmIdx, 4 );
    cornerIndices[5] = m_femPart->elementNodeResultIdx( elmIdx, 5 );
    cornerIndices[6] = m_femPart->elementNodeResultIdx( elmIdx, 6 );
    cornerIndices[7] = m_femPart->elementNodeResultIdx( elmIdx, 7 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigFault* RivFemIntersectionGrid::findFaultFromCellIndexAndCellFace( size_t reservoirCellIndex,
                                                                           cvf::StructGridInterface::FaceType face ) const
{
    return nullptr;
}
