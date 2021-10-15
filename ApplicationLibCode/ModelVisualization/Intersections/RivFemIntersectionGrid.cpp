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
    for ( int i = 0; i < m_femParts->partCount(); i++ )
    {
        const RigFemPart*   part = m_femParts->part( i );
        std::vector<size_t> foundElements;
        part->findIntersectingCells( intersectingBB, &foundElements );

        for ( size_t t = 0; t < foundElements.size(); t++ )
        {
            size_t globalIdx = m_femParts->globalIndex( i, foundElements[t] );
            intersectedCells->push_back( globalIdx );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivFemIntersectionGrid::useCell( size_t globalCellIndex ) const
{
    auto [part, elementIdx] = m_femParts->partAndElementIndex( globalCellIndex );

    return ( ( part->elementType( elementIdx ) == RigElementType::HEX8 ) ||
             ( part->elementType( elementIdx ) == RigElementType::HEX8P ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivFemIntersectionGrid::cellCornerVertices( size_t globalCellIndex, cvf::Vec3d cellCorners[8] ) const
{
    auto [part, elementIdx] = m_femParts->partAndElementIndex( globalCellIndex );

    const std::vector<cvf::Vec3f>& nodeCoords    = part->nodes().coordinates;
    const int*                     cornerIndices = part->connectivities( elementIdx );

    for ( int i = 0; i < 8; i++ )
    {
        cellCorners[i] = cvf::Vec3d( nodeCoords[cornerIndices[i]] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivFemIntersectionGrid::cellCornerIndices( size_t globalCellIndex, size_t cornerIndices[8] ) const
{
    auto [part, elementIdx] = m_femParts->partAndElementIndex( globalCellIndex );

    RigElementType elmType = part->elementType( elementIdx );
    if ( !( elmType == HEX8 || elmType == HEX8P ) ) return;

    int       elmIdx = static_cast<int>( elementIdx );
    const int partId = part->elementPartId();

    for ( int i = 0; i < 8; i++ )
    {
        cornerIndices[i] = m_femParts->globalElementNodeResultIdx( partId, elmIdx, i );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigFault* RivFemIntersectionGrid::findFaultFromCellIndexAndCellFace( size_t reservoirCellIndex,
                                                                           cvf::StructGridInterface::FaceType face ) const
{
    return nullptr;
}
