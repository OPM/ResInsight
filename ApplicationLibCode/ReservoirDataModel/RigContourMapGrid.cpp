/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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

#include "RigContourMapGrid.h"

#include <cmath>

using namespace cvf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigContourMapGrid::RigContourMapGrid( const cvf::BoundingBox& originalBoundingBox, double sampleSpacing )
    : m_sampleSpacing( sampleSpacing )
    , m_mapSize( cvf::Vec2ui( 0u, 0u ) )
    , m_originalBoundingBox( originalBoundingBox )
{
    cvf::Vec3d minExpandedPoint = originalBoundingBox.min() - cvf::Vec3d( gridEdgeOffset(), gridEdgeOffset(), 0.0 );
    cvf::Vec3d maxExpandedPoint = originalBoundingBox.max() + cvf::Vec3d( gridEdgeOffset(), gridEdgeOffset(), 0.0 );
    m_expandedBoundingBox       = cvf::BoundingBox( minExpandedPoint, maxExpandedPoint );

    m_mapSize = calculateMapSize( m_expandedBoundingBox.extent(), sampleSpacing );

    // Re-jig max point to be an exact multiple of cell size
    m_expandedBoundingBox = makeMaxPointMultipleOfCellSize( m_expandedBoundingBox, m_mapSize, sampleSpacing );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigContourMapGrid::RigContourMapGrid( const cvf::BoundingBox& originalBoundingBox, const cvf::BoundingBox& expandedBoundingBox, double sampleSpacing )
    : m_sampleSpacing( sampleSpacing )
    , m_mapSize( cvf::Vec2ui( 0u, 0u ) )
    , m_originalBoundingBox( originalBoundingBox )
    , m_expandedBoundingBox( expandedBoundingBox )
{
    m_mapSize = calculateMapSize( m_expandedBoundingBox.extent(), sampleSpacing );

    // Re-jig max point to be an exact multiple of cell size
    m_expandedBoundingBox = makeMaxPointMultipleOfCellSize( m_expandedBoundingBox, m_mapSize, sampleSpacing );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigContourMapGrid::sampleSpacing() const
{
    return m_sampleSpacing;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui RigContourMapGrid::numberOfElementsIJ() const
{
    return m_mapSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui RigContourMapGrid::numberOfVerticesIJ() const
{
    cvf::Vec2ui mapSize = numberOfElementsIJ();
    mapSize.x() += 1u;
    mapSize.y() += 1u;
    return mapSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
uint RigContourMapGrid::numberOfCells() const
{
    return m_mapSize.x() * m_mapSize.y();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigContourMapGrid::numberOfVertices() const
{
    cvf::Vec2ui gridSize = numberOfVerticesIJ();
    return static_cast<size_t>( gridSize.x() ) * static_cast<size_t>( gridSize.y() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigContourMapGrid::origin3d() const
{
    return m_expandedBoundingBox.min();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RigContourMapGrid::generateVertices() const
{
    size_t                  nVertices = numberOfVertices();
    std::vector<cvf::Vec3d> vertices( nVertices, cvf::Vec3d::ZERO );

#pragma omp parallel for
    for ( int index = 0; index < static_cast<int>( nVertices ); ++index )
    {
        cvf::Vec2ui ij     = ijFromVertexIndex( index );
        cvf::Vec2d  mapPos = cellCenterPosition( ij.x(), ij.y() );
        // Shift away from sample point to vertex
        mapPos.x() -= sampleSpacing() * 0.5;
        mapPos.y() -= sampleSpacing() * 0.5;

        cvf::Vec3d vertexPos( mapPos, 0.0 );
        vertices[index] = vertexPos;
    }
    return vertices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui RigContourMapGrid::ijFromVertexIndex( size_t gridIndex ) const
{
    cvf::Vec2ui gridSize = numberOfVerticesIJ();

    uint quotientX  = static_cast<uint>( gridIndex ) / gridSize.x();
    uint remainderX = static_cast<uint>( gridIndex ) % gridSize.x();

    return cvf::Vec2ui( remainderX, quotientX );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigContourMapGrid::cellIndexFromIJ( uint i, uint j ) const
{
    CVF_ASSERT( i < m_mapSize.x() );
    CVF_ASSERT( j < m_mapSize.y() );

    return i + j * m_mapSize.x();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui RigContourMapGrid::ijFromCellIndex( size_t cellIndex ) const
{
    CVF_TIGHT_ASSERT( cellIndex < numberOfCells() );

    uint quotientX  = static_cast<uint>( cellIndex ) / m_mapSize.x();
    uint remainderX = static_cast<uint>( cellIndex ) % m_mapSize.x();

    return cvf::Vec2ui( remainderX, quotientX );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui RigContourMapGrid::ijFromLocalPos( const cvf::Vec2d& localPos2d ) const
{
    uint i = static_cast<uint>( localPos2d.x() / sampleSpacing() );
    uint j = static_cast<uint>( localPos2d.y() / sampleSpacing() );
    return cvf::Vec2ui( i, j );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2d RigContourMapGrid::cellCenterPosition( uint i, uint j ) const
{
    cvf::Vec3d gridExtent = m_expandedBoundingBox.extent();
    cvf::Vec2d cellCorner = cvf::Vec2d( ( i * gridExtent.x() ) / ( m_mapSize.x() ), ( j * gridExtent.y() ) / ( m_mapSize.y() ) );

    return cellCorner + cvf::Vec2d( sampleSpacing() * 0.5, sampleSpacing() * 0.5 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2d RigContourMapGrid::origin2d() const
{
    return cvf::Vec2d( m_expandedBoundingBox.min().x(), m_expandedBoundingBox.min().y() );
}

//--------------------------------------------------------------------------------------------------
/// Vertex positions in local coordinates (add origin2d.x() for UTM x)
//--------------------------------------------------------------------------------------------------
std::vector<double> RigContourMapGrid::xVertexPositions() const
{
    double gridExtent = m_expandedBoundingBox.extent().x();

    cvf::Vec2ui         gridSize = numberOfVerticesIJ();
    std::vector<double> positions;
    positions.reserve( gridSize.x() );
    for ( uint i = 0; i < gridSize.x(); ++i )
    {
        positions.push_back( ( i * gridExtent ) / ( gridSize.x() - 1 ) );
    }

    return positions;
}

//--------------------------------------------------------------------------------------------------
/// Vertex positions in local coordinates (add origin2d.y() for UTM y)
//--------------------------------------------------------------------------------------------------
std::vector<double> RigContourMapGrid::yVertexPositions() const
{
    double gridExtent = m_expandedBoundingBox.extent().y();

    cvf::Vec2ui         gridSize = numberOfVerticesIJ();
    std::vector<double> positions;
    positions.reserve( gridSize.y() );
    for ( uint j = 0; j < gridSize.y(); ++j )
    {
        positions.push_back( ( j * gridExtent ) / ( gridSize.y() - 1 ) );
    }

    return positions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigContourMapGrid::gridEdgeOffset() const
{
    return sampleSpacing() * 2.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::BoundingBox& RigContourMapGrid::expandedBoundingBox() const
{
    return m_expandedBoundingBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::BoundingBox& RigContourMapGrid::originalBoundingBox() const
{
    return m_originalBoundingBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::Vec2ui& RigContourMapGrid::mapSize() const
{
    return m_mapSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigContourMapGrid::vertexIndexFromIJ( uint i, uint j ) const
{
    return i + j * ( m_mapSize.x() + 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui RigContourMapGrid::calculateMapSize( const cvf::Vec3d& gridExtent, double sampleSpacing )
{
    uint projectionSizeX = static_cast<uint>( std::ceil( gridExtent.x() / sampleSpacing ) );
    uint projectionSizeY = static_cast<uint>( std::ceil( gridExtent.y() / sampleSpacing ) );

    return cvf::Vec2ui( projectionSizeX, projectionSizeY );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox
    RigContourMapGrid::makeMaxPointMultipleOfCellSize( const cvf::BoundingBox& boundingBox, const cvf::Vec2ui& mapSize, double sampleSpacing )
{
    cvf::Vec3d minPoint = boundingBox.min();
    cvf::Vec3d maxPoint = boundingBox.max();
    maxPoint.x()        = minPoint.x() + mapSize.x() * sampleSpacing;
    maxPoint.y()        = minPoint.y() + mapSize.y() * sampleSpacing;
    return cvf::BoundingBox( minPoint, maxPoint );
}
