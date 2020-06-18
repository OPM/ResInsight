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

#include "cvfDebugTimer.h"
#include "cvfGeometryBuilderDrawableGeo.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfScalarMapper.h"

#include "cvfArray.h"
#include "cvfOutlineEdgeExtractor.h"
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
    if ( m_includeRanges.size() == 0 )
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
    if ( m_includeRanges.size() > 0 )
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
    : m_grid( grid )
    , m_useOpenMP( useOpenMP )
{
    CVF_ASSERT( grid );
    m_quadMapper     = new StructGridQuadToCellFaceMapper;
    m_triangleMapper = new StuctGridTriangleToCellFaceMapper( m_quadMapper.p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
StructGridGeometryGenerator::~StructGridGeometryGenerator()
{
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

    ref<UIntArray>               indices = lineIndicesFromQuadVertexArray( m_vertices.p() );
    ref<PrimitiveSetIndexedUInt> prim    = new PrimitiveSetIndexedUInt( PT_LINES );
    prim->setIndices( indices.p() );

    geo->addPrimitiveSet( prim.p() );
    return geo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> StructGridGeometryGenerator::createOutlineMeshDrawable( double creaseAngle )
{
    if ( !( m_vertices.notNull() && m_vertices->size() != 0 ) ) return nullptr;

    cvf::OutlineEdgeExtractor ee( creaseAngle, *m_vertices );

    ref<UIntArray> indices = lineIndicesFromQuadVertexArray( m_vertices.p() );
    ee.addPrimitives( 4, *indices );

    ref<cvf::UIntArray> lineIndices = ee.lineIndices();
    if ( lineIndices->size() == 0 )
    {
        return nullptr;
    }

    ref<PrimitiveSetIndexedUInt> prim = new PrimitiveSetIndexedUInt( PT_LINES );
    prim->setIndices( lineIndices.p() );

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray( m_vertices.p() );
    geo->addPrimitiveSet( prim.p() );

    return geo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> StructGridGeometryGenerator::createMeshDrawableFromSingleCell( const StructGridInterface* grid,
                                                                                size_t                     cellIndex )
{
    return createMeshDrawableFromSingleCell( grid, cellIndex, grid->displayModelOffset() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> StructGridGeometryGenerator::createMeshDrawableFromSingleCell( const StructGridInterface* grid,
                                                                                size_t                     cellIndex,
                                                                                const cvf::Vec3d& displayModelOffset )
{
    cvf::Vec3d cornerVerts[8];
    grid->cellCornerVertices( cellIndex, cornerVerts );

    std::vector<Vec3f> vertices;

    for ( int enumInt = cvf::StructGridInterface::POS_I; enumInt < cvf::StructGridInterface::NO_FACE; enumInt++ )
    {
        cvf::StructGridInterface::FaceType face = static_cast<cvf::StructGridInterface::FaceType>( enumInt );

        ubyte faceConn[4];
        grid->cellFaceVertexIndices( face, faceConn );

        int n;
        for ( n = 0; n < 4; n++ )
        {
            vertices.push_back( cvf::Vec3f( cornerVerts[faceConn[n]] - displayModelOffset ) );
        }
    }

    cvf::ref<cvf::Vec3fArray> cvfVertices = new cvf::Vec3fArray;
    cvfVertices->assign( vertices );

    if ( !( cvfVertices.notNull() && cvfVertices->size() != 0 ) ) return nullptr;

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray( cvfVertices.p() );

    ref<UIntArray>               indices = lineIndicesFromQuadVertexArray( cvfVertices.p() );
    ref<PrimitiveSetIndexedUInt> prim    = new PrimitiveSetIndexedUInt( PT_LINES );
    prim->setIndices( indices.p() );

    geo->addPrimitiveSet( prim.p() );
    return geo;
}

//--------------------------------------------------------------------------------------------------
///
///
///
///
//--------------------------------------------------------------------------------------------------
ref<UIntArray> StructGridGeometryGenerator::lineIndicesFromQuadVertexArray( const Vec3fArray* vertexArray )
{
    CVF_ASSERT( vertexArray );

    size_t numVertices = vertexArray->size();
    int    numQuads    = static_cast<int>( numVertices / 4 );
    CVF_ASSERT( numVertices % 4 == 0 );

    ref<UIntArray> indices = new UIntArray;
    indices->resize( numQuads * 8 );

#pragma omp parallel for
    for ( int i = 0; i < numQuads; i++ )
    {
        int idx = 8 * i;
        indices->set( idx + 0, i * 4 + 0 );
        indices->set( idx + 1, i * 4 + 1 );
        indices->set( idx + 2, i * 4 + 1 );
        indices->set( idx + 3, i * 4 + 2 );
        indices->set( idx + 4, i * 4 + 2 );
        indices->set( idx + 5, i * 4 + 3 );
        indices->set( idx + 6, i * 4 + 3 );
        indices->set( idx + 7, i * 4 + 0 );
    }

    return indices;
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

                if ( visibleFaces.size() > 0 )
                {
                    cvf::Vec3d cornerVerts[8];
                    m_grid->cellCornerVertices( cellIndex, cornerVerts );

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

#if 0
//--------------------------------------------------------------------------------------------------
/// 
/// 
//--------------------------------------------------------------------------------------------------
void StructGridGeometryGenerator::textureCoordinatesFromSingleFaceValues(Vec2fArray* textureCoords, const ScalarMapper* mapper, const CellFaceValueCalculator* resultAccessor) const
{
    if (!resultAccessor) return;

    textureCoords->resize(m_quadMapper->quadCount()*4);

    cvf::Vec2f* rawPtr = textureCoords->ptr();

    double cellFaceValue;
    cvf::Vec2f texCoord;
    int quadCount = static_cast<int>(m_quadMapper->quadCount());

#pragma omp parallel for private( texCoord, cellFaceValue )
    for (int qIdx = 0; qIdx < quadCount; qIdx++)
    {
        cellFaceValue = resultAccessor->cellFaceScalar(m_quadMapper->cellIndex(qIdx), m_quadMapper->faceType(qIdx));
        
        texCoord = mapper->mapToTextureCoord(cellFaceValue);

        if (cellFaceValue == HUGE_VAL || cellFaceValue != cellFaceValue) // a != a is true for NAN's
        {
            texCoord[1] = 1.0f;
        }

        size_t j;
        for (j = 0; j < 4; j++)
        {   
            rawPtr[qIdx*4 + j] = texCoord;
        }
    }
}
#endif

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void StructGridGeometryGenerator::setCellVisibility( const UByteArray* cellVisibility )
{
    m_cellVisibility = cellVisibility;
}

} // namespace cvf
