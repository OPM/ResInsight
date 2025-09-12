//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include "cvfBase.h"

#include "cvfStructGrid.h"
#include "cvfStructGridGeometryGenerator.h"
#include "cvfStructGridScalarDataAccess.h"
#include "cvfStructGridTools.h"

#include "cvfDebugTimer.h"
#include "cvfGeometryBuilderDrawableGeo.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfScalarMapper.h"

#include "cvfArray.h"
#include "cvfOutlineEdgeExtractor.h"

#include <array>
#include <cmath>

namespace cvf
{
//==================================================================================================
///
/// \class CellRangeFilter
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CellRangeFilter::CellRangeFilter()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CellRangeFilter::addCellIncludeRange( size_t minI,
                                           size_t minJ,
                                           size_t minK,
                                           size_t maxI,
                                           size_t maxJ,
                                           size_t maxK,
                                           bool   applyToSubGridAreas )
{
    m_includeRanges.push_back( CellRange( minI, minJ, minK, maxI, maxJ, maxK, applyToSubGridAreas ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CellRangeFilter::addCellInclude( size_t i, size_t j, size_t k, bool applyToSubGridAreas )
{
    m_includeRanges.push_back( CellRange( i, j, k, i + 1, j + 1, k + 1, applyToSubGridAreas ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CellRangeFilter::addCellExcludeRange( size_t minI,
                                           size_t minJ,
                                           size_t minK,
                                           size_t maxI,
                                           size_t maxJ,
                                           size_t maxK,
                                           bool   applyToSubGridAreas )
{
    m_excludeRanges.push_back( CellRange( minI, minJ, minK, maxI, maxJ, maxK, applyToSubGridAreas ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CellRangeFilter::addCellExclude( size_t i, size_t j, size_t k, bool applyToSubGridAreas )
{
    m_excludeRanges.push_back( CellRange( i, j, k, i + 1, j + 1, k + 1, applyToSubGridAreas ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool CellRangeFilter::isCellVisible( size_t i, size_t j, size_t k, bool isInSubGridArea ) const
{
    if ( m_includeRanges.empty() )
    {
        return false;
    }

    size_t idx;
    for ( idx = 0; idx < m_excludeRanges.size(); idx++ )
    {
        if ( m_excludeRanges[idx].isInRange( i, j, k, isInSubGridArea ) )
        {
            return false;
        }
    }

    for ( idx = 0; idx < m_includeRanges.size(); idx++ )
    {
        if ( m_includeRanges[idx].isInRange( i, j, k, isInSubGridArea ) )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool CellRangeFilter::isCellExcluded( size_t i, size_t j, size_t k, bool isInSubGridArea ) const
{
    for ( size_t idx = 0; idx < m_excludeRanges.size(); idx++ )
    {
        if ( m_excludeRanges[idx].isInRange( i, j, k, isInSubGridArea ) )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool CellRangeFilter::hasIncludeRanges() const
{
    if ( !m_includeRanges.empty() )
        return true;
    else
        return false;
}

//==================================================================================================
///
/// \class cvf::StructGridGeometry
/// \ingroup StructGrid
///
///
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
StructGridGeometryGenerator::StructGridGeometryGenerator( const StructGridInterface* grid, bool useOpenMP )
    : GeometryGeneratorInterface( grid, useOpenMP )
{
    CVF_ASSERT( grid );
    m_quadMapper     = new StructGridQuadToCellFaceMapper;
    m_triangleMapper = new StuctGridTriangleToCellFaceMapper( m_quadMapper.p() );
}

//--------------------------------------------------------------------------------------------------
/// Generate surface drawable geo from the specified region
///
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> StructGridGeometryGenerator::generateSurface()
{
    computeArrays();

    CVF_ASSERT( m_vertices.notNull() );

    if ( m_vertices->size() == 0 ) return nullptr;

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setFromQuadVertexArray( m_vertices.p() );

    return geo;
}

//--------------------------------------------------------------------------------------------------
/// Generates simplified mesh as line drawing
/// Must call generateSurface first
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> StructGridGeometryGenerator::createMeshDrawable()
{
    if ( !( m_vertices.notNull() && m_vertices->size() != 0 ) ) return nullptr;

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray( m_vertices.p() );

    ref<UIntArray>               indices = StructGridTools::lineIndicesFromQuadVertexArray( m_vertices.p() );
    ref<PrimitiveSetIndexedUInt> prim    = new PrimitiveSetIndexedUInt( PT_LINES );
    prim->setIndices( indices.p() );

    geo->addPrimitiveSet( prim.p() );
    return geo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::GridGeometryType StructGridGeometryGenerator::geometryType() const
{
    return GridGeometryType::HEXAHEDRAL;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void StructGridGeometryGenerator::addFaceVisibilityFilter( const CellFaceVisibilityFilter* cellVisibilityFilter )
{
    m_cellVisibilityFilters.push_back( cellVisibilityFilter );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool StructGridGeometryGenerator::isCellFaceVisible( size_t i, size_t j, size_t k, StructGridInterface::FaceType face ) const
{
    size_t idx;
    for ( idx = 0; idx < m_cellVisibilityFilters.size(); idx++ )
    {
        const cvf::CellFaceVisibilityFilter* cellFilter = m_cellVisibilityFilters[idx];
        if ( cellFilter->isFaceVisible( i, j, k, face, m_cellVisibility.p() ) )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void StructGridGeometryGenerator::computeArrays()
{
    std::vector<Vec3f> vertices;
    m_quadMapper->quadToCellIndexMap().clear();
    m_quadMapper->quadToCellFaceMap().clear();

    cvf::Vec3d offset = m_grid->displayModelOffset();

#pragma omp parallel for schedule( dynamic ) if ( m_useOpenMP )
    for ( int k = 0; k < static_cast<int>( m_grid->cellCountK() ); k++ )
    {
        size_t j;
        for ( j = 0; j < m_grid->cellCountJ(); j++ )
        {
            size_t i;
            for ( i = 0; i < m_grid->cellCountI(); i++ )
            {
                size_t cellIndex = m_grid->cellIndexFromIJK( i, j, k );
                if ( m_cellVisibility.notNull() && !( *m_cellVisibility )[cellIndex] )
                {
                    continue;
                }

                std::vector<StructGridInterface::FaceType> visibleFaces;
                visibleFaces.reserve( 6 );

                if ( isCellFaceVisible( i, j, k, StructGridInterface::NEG_I ) )
                    visibleFaces.push_back( cvf::StructGridInterface::NEG_I );
                if ( isCellFaceVisible( i, j, k, StructGridInterface::POS_I ) )
                    visibleFaces.push_back( cvf::StructGridInterface::POS_I );
                if ( isCellFaceVisible( i, j, k, StructGridInterface::NEG_J ) )
                    visibleFaces.push_back( cvf::StructGridInterface::NEG_J );
                if ( isCellFaceVisible( i, j, k, StructGridInterface::POS_J ) )
                    visibleFaces.push_back( cvf::StructGridInterface::POS_J );
                if ( isCellFaceVisible( i, j, k, StructGridInterface::NEG_K ) )
                    visibleFaces.push_back( cvf::StructGridInterface::NEG_K );
                if ( isCellFaceVisible( i, j, k, StructGridInterface::POS_K ) )
                    visibleFaces.push_back( cvf::StructGridInterface::POS_K );

                if ( !visibleFaces.empty() )
                {
                    std::array<cvf::Vec3d, 8> cornerVerts = m_grid->cellCornerVertices( cellIndex );

                    size_t idx;
                    for ( idx = 0; idx < visibleFaces.size(); idx++ )
                    {
                        cvf::StructGridInterface::FaceType face = visibleFaces[idx];

                        ubyte faceConn[4];
                        m_grid->cellFaceVertexIndices( face, faceConn );

// Critical section to avoid two threads accessing the arrays at the same time.
#pragma omp critical( critical_section_StructGridGeometryGenerator_computeArrays )
                        {
                            int n;
                            for ( n = 0; n < 4; n++ )
                            {
                                vertices.push_back( cvf::Vec3f( cornerVerts[faceConn[n]] - offset ) );
                            }

                            // Keep track of the source cell index per quad
                            m_quadMapper->quadToCellIndexMap().push_back( cellIndex );
                            m_quadMapper->quadToCellFaceMap().push_back( face );
                        }
                    }
                }
            }
        }
    }

    m_vertices = new cvf::Vec3fArray;
    m_vertices->assign( vertices );
}

//--------------------------------------------------------------------------------------------------
/// Calculates the texture coordinates in a "nearly" one dimentional texture.
/// Undefined values are coded with a y-texturecoordinate value of 1.0 instead of the normal 0.5
//--------------------------------------------------------------------------------------------------
void StructGridGeometryGenerator::textureCoordinates( Vec2fArray*                       textureCoords,
                                                      const StructGridScalarDataAccess* resultAccessor,
                                                      const ScalarMapper*               mapper ) const
{
    if ( !resultAccessor ) return;

    size_t numVertices = m_quadMapper->quadCount() * 4;

    textureCoords->resize( numVertices );
    cvf::Vec2f* rawPtr = textureCoords->ptr();

    double     cellScalarValue;
    cvf::Vec2f texCoord;

#pragma omp parallel for private( texCoord, cellScalarValue )
    for ( int i = 0; i < static_cast<int>( m_quadMapper->quadCount() ); i++ )
    {
        cellScalarValue = resultAccessor->cellScalar( m_quadMapper->cellIndex( i ) );
        texCoord        = mapper->mapToTextureCoord( cellScalarValue );
        if ( cellScalarValue == HUGE_VAL || cellScalarValue != cellScalarValue ) // a != a is true for NAN's
        {
            texCoord[1] = 1.0f;
        }

        size_t j;
        for ( j = 0; j < 4; j++ )
        {
            rawPtr[i * 4 + j] = texCoord;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::StructGridQuadToCellFaceMapper* StructGridGeometryGenerator::quadToCellFaceMapper() const
{
    return m_quadMapper.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::StuctGridTriangleToCellFaceMapper* StructGridGeometryGenerator::triangleToCellFaceMapper() const
{
    return m_triangleMapper.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void StructGridGeometryGenerator::setCellVisibility( const UByteArray* cellVisibility )
{
    m_cellVisibility = cellVisibility;
}

} // namespace cvf
