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

#include "RimGeoMechPartCollection.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivFemIntersectionGrid::RivFemIntersectionGrid( const RigFemPartCollection* femParts, const RimGeoMechPartCollection* parts )
    : m_femParts( femParts )
    , m_parts( parts )
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
std::vector<size_t> RivFemIntersectionGrid::findIntersectingCells( const cvf::BoundingBox& intersectingBB ) const
{
    // For FEM models the term element is used instead of cell.
    // Each FEM part has a local element index which is transformed into global index for a FEM part collection.
    return m_femParts->findIntersectingGlobalElementIndices( intersectingBB );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivFemIntersectionGrid::useCell( size_t globalCellIndex ) const
{
    auto [part, elementIdx] = m_femParts->partAndElementIndex( globalCellIndex );

    return ( ( part->elementType( elementIdx ) == RigElementType::HEX8 ) || ( part->elementType( elementIdx ) == RigElementType::HEX8P ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivFemIntersectionGrid::cellCornerVertices( size_t globalCellIndex, std::array<cvf::Vec3d, 8>& cellCorners ) const
{
    auto [part, elementIdx] = m_femParts->partAndElementIndex( globalCellIndex );

    const bool useDisplacements = m_parts->isDisplacementsUsed();

    const std::vector<cvf::Vec3f>& nodeCoords    = part->nodes().coordinates;
    const int*                     cornerIndices = part->connectivities( elementIdx );

    if ( useDisplacements )
    {
        const double                   scaleFactor   = m_parts->currentDisplacementScaleFactor();
        const std::vector<cvf::Vec3f>& displacements = m_parts->displacements( part->elementPartId() );
        for ( int i = 0; i < 8; i++ )
        {
            const int idx  = cornerIndices[i];
            cellCorners[i] = cvf::Vec3d( nodeCoords[idx] + displacements[idx] * scaleFactor );
        }
    }
    else
    {
        for ( int i = 0; i < 8; i++ )
        {
            cellCorners[i] = cvf::Vec3d( nodeCoords[cornerIndices[i]] );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivFemIntersectionGrid::cellCornerIndices( size_t globalCellIndex, std::array<size_t, 8>& cornerIndices ) const
{
    auto [part, elementIdx] = m_femParts->partAndElementIndex( globalCellIndex );

    RigElementType elmType = part->elementType( elementIdx );
    if ( !RigFemTypes::is8NodeElement( elmType ) ) return;

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
const RigFault* RivFemIntersectionGrid::findFaultFromCellIndexAndCellFace( size_t                             reservoirCellIndex,
                                                                           cvf::StructGridInterface::FaceType face ) const
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivFemIntersectionGrid::setKIntervalFilter( bool enabled, std::string kIntervalStr )
{
    // not supported for geomech grids
}
