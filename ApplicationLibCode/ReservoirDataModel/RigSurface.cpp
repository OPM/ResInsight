/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RigSurface.h"

#include "cafAssert.h"
#include "cvfBoundingBox.h"
#include "cvfBoundingBoxTree.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigSurface::RigSurface() = default;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigSurface::~RigSurface() = default;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<unsigned>& RigSurface::triangleIndices() const
{
    return m_triangleIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::Vec3d>& RigSurface::vertices() const
{
    return m_vertices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSurface::setTriangleData( const std::vector<unsigned>& tringleIndices, const std::vector<cvf::Vec3d>& vertices )
{
    m_triangleIndices = tringleIndices;
    m_vertices        = vertices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSurface::addVerticeResult( const QString resultName, const std::vector<float>& resultValues )
{
    m_verticeResults[resultName] = resultValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<float> RigSurface::propertyValues( const QString& propertyName ) const
{
    auto it = m_verticeResults.find( propertyName );
    if ( it != m_verticeResults.end() )
    {
        return it->second;
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RigSurface::propertyNames() const
{
    std::vector<QString> names;

    for ( const auto& propertyResult : m_verticeResults )
    {
        names.push_back( propertyResult.first );
    }

    return names;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSurface::findIntersectingTriangles( const cvf::BoundingBox& inputBB, std::vector<size_t>* triangleStartIndices ) const
{
    CAF_ASSERT( m_surfaceBoundingBoxTree.notNull() );

    m_surfaceBoundingBoxTree->findIntersections( inputBB, triangleStartIndices );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSurface::ensureIntersectionSearchTreeIsBuilt()
{
    if ( m_surfaceBoundingBoxTree.isNull() )
    {
        size_t itemCount = triangleCount();

        std::vector<cvf::BoundingBox> cellBoundingBoxes;
        std::vector<size_t>           boundingBoxIds;
        cellBoundingBoxes.resize( itemCount );
        boundingBoxIds.resize( itemCount );

        for ( size_t triangleIdx = 0; triangleIdx < itemCount; ++triangleIdx )
        {
            cvf::BoundingBox& cellBB = cellBoundingBoxes[triangleIdx];
            cellBB.add( m_vertices[m_triangleIndices[triangleIdx * 3 + 0]] );
            cellBB.add( m_vertices[m_triangleIndices[triangleIdx * 3 + 1]] );
            cellBB.add( m_vertices[m_triangleIndices[triangleIdx * 3 + 2]] );

            boundingBoxIds[triangleIdx] = triangleIdx * 3;
        }

        m_surfaceBoundingBoxTree = new cvf::BoundingBoxTree;
        m_surfaceBoundingBoxTree->buildTreeFromBoundingBoxes( cellBoundingBoxes, &boundingBoxIds );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigSurface::triangleCount() const
{
    return m_triangleIndices.size() / 3;
}
