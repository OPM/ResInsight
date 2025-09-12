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

#include "cvfCylindricalGeometryGenerator.h"

#include "cvfArray.h"
#include "cvfBase.h"
#include "cvfDrawableGeo.h"
#include "cvfGeometryBuilderDrawableGeo.h"
#include "cvfOutlineEdgeExtractor.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfScalarMapper.h"
#include "cvfStructGridGeometryGenerator.h"
#include "cvfStructGridScalarDataAccess.h"
#include "cvfStructGridTools.h"

#include <cmath>
#include <numbers>

namespace cvf
{

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CylindricalGeometryGenerator::CylindricalGeometryGenerator( const StructGridInterface* grid, bool useOpenMP )
    : GeometryGeneratorInterface( grid, useOpenMP )
    , m_curveSubdivisions( 10 )
{
    m_quadMapper     = new StructGridQuadToCellFaceMapper;
    m_triangleMapper = new StuctGridTriangleToCellFaceMapper( m_quadMapper.p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CylindricalGeometryGenerator::setCellVisibility( const UByteArray* cellVisibility )
{
    m_cellVisibility = cellVisibility;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CylindricalGeometryGenerator::addFaceVisibilityFilter( const CellFaceVisibilityFilter* cellVisibilityFilter )
{
    m_cellVisibilityFilters.push_back( cellVisibilityFilter );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CylindricalGeometryGenerator::setCurveSubdivisions( int subdivisions )
{
    m_curveSubdivisions = subdivisions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> CylindricalGeometryGenerator::generateSurface()
{
    computeArrays();

    CVF_ASSERT( m_vertices.notNull() );

    if ( m_vertices->size() == 0 ) return nullptr;

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setFromQuadVertexArray( m_vertices.p() );

    return geo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> CylindricalGeometryGenerator::createMeshDrawable()
{
    if ( !( m_vertices.notNull() && m_vertices->size() != 0 ) ) return nullptr;

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray( m_vertices.p() );

    // Use cached mesh line indices if available, otherwise compute them
    if ( m_meshLineIndices.isNull() )
    {
        m_meshLineIndices = createCylindricalMeshLineIndices();
    }

    ref<PrimitiveSetIndexedUInt> prim = new PrimitiveSetIndexedUInt( PT_LINES );
    prim->setIndices( m_meshLineIndices.p() );

    geo->addPrimitiveSet( prim.p() );
    return geo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::GridGeometryType CylindricalGeometryGenerator::geometryType() const
{
    return GridGeometryType::CYLINDRICAL;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CylindricalGeometryGenerator::textureCoordinates( Vec2fArray*                       textureCoords,
                                                       const StructGridScalarDataAccess* resultAccessor,
                                                       const ScalarMapper*               mapper ) const
{
    if ( !resultAccessor ) return;

    size_t numVertices = m_quadMapper->quadCount() * 4;

    textureCoords->resize( numVertices );
    cvf::Vec2f* rawPtr = textureCoords->ptr();

    double     cellScalarValue;
    cvf::Vec2f texCoord;

#pragma omp parallel for private( texCoord, cellScalarValue ) if ( m_useOpenMP )
    for ( int i = 0; i < static_cast<int>( m_quadMapper->quadCount() ); i++ )
    {
        cellScalarValue = resultAccessor->cellScalar( m_quadMapper->cellIndex( i ) );
        texCoord        = mapper->mapToTextureCoord( cellScalarValue );
        if ( cellScalarValue == HUGE_VAL || cellScalarValue != cellScalarValue )
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
const StructGridQuadToCellFaceMapper* CylindricalGeometryGenerator::quadToCellFaceMapper() const
{
    return m_quadMapper.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const StuctGridTriangleToCellFaceMapper* CylindricalGeometryGenerator::triangleToCellFaceMapper() const
{
    return m_triangleMapper.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CylindricalGeometryGenerator::computeArrays()
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

                auto cylResult = m_grid->getCylindricalCoords( cellIndex );
                if ( !cylResult.has_value() )
                {
                    continue;
                }
                const cvf::CylindricalCell& cylCell = cylResult.value();

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
#pragma omp critical( critical_section_CylindricalGeometryGenerator_computeArrays )
                    {
                        generateCylindricalQuads( cylCell, cellIndex, vertices );
                    }
                }
            }
        }
    }

    m_vertices = new cvf::Vec3fArray;
    m_vertices->assign( vertices );

    // Invalidate cached mesh line indices when vertices change
    m_meshLineIndices = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CylindricalGeometryGenerator::generateCylindricalQuads( const cvf::CylindricalCell& cell,
                                                             size_t                      cellIndex,
                                                             std::vector<Vec3f>&         vertices )
{
    addRadialFaces( cell, cellIndex, vertices );
    addCircumferentialFaces( cell, cellIndex, vertices );
    addTopBottomFaces( cell, cellIndex, vertices );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CylindricalGeometryGenerator::addRadialFaces( const cvf::CylindricalCell& cell,
                                                   size_t                      cellIndex,
                                                   std::vector<Vec3f>&         vertices )
{
    cvf::Vec3d offset = m_grid->displayModelOffset();

    // Create angular subdivisions for smooth curved surfaces
    double angleRange = cell.endAngle - cell.startAngle;
    double angleStep  = angleRange / m_curveSubdivisions;

    // Inner radial face (NEG_I) - angular subdivision only
    for ( int i = 0; i < m_curveSubdivisions; ++i )
    {
        double angle1 = cell.startAngle + i * angleStep;
        double angle2 = cell.startAngle + ( i + 1 ) * angleStep;

        cvf::Vec3d innerBot1 = StructGridTools::cylindricalToCartesian( cell.innerRadius, angle1, cell.bottomZ );
        cvf::Vec3d innerBot2 = StructGridTools::cylindricalToCartesian( cell.innerRadius, angle2, cell.bottomZ );
        cvf::Vec3d innerTop1 = StructGridTools::cylindricalToCartesian( cell.innerRadius, angle1, cell.topZ );
        cvf::Vec3d innerTop2 = StructGridTools::cylindricalToCartesian( cell.innerRadius, angle2, cell.topZ );

        // Create quad for curved surface
        vertices.push_back( cvf::Vec3f( innerBot1 - offset ) );
        vertices.push_back( cvf::Vec3f( innerBot2 - offset ) );
        vertices.push_back( cvf::Vec3f( innerTop2 - offset ) );
        vertices.push_back( cvf::Vec3f( innerTop1 - offset ) );

        m_quadMapper->quadToCellIndexMap().push_back( cellIndex );
        m_quadMapper->quadToCellFaceMap().push_back( StructGridInterface::NEG_I );
    }

    // Outer radial face (POS_I) - angular subdivision only
    for ( int i = 0; i < m_curveSubdivisions; ++i )
    {
        double angle1 = cell.startAngle + i * angleStep;
        double angle2 = cell.startAngle + ( i + 1 ) * angleStep;

        cvf::Vec3d outerBot1 = StructGridTools::cylindricalToCartesian( cell.outerRadius, angle1, cell.bottomZ );
        cvf::Vec3d outerBot2 = StructGridTools::cylindricalToCartesian( cell.outerRadius, angle2, cell.bottomZ );
        cvf::Vec3d outerTop1 = StructGridTools::cylindricalToCartesian( cell.outerRadius, angle1, cell.topZ );
        cvf::Vec3d outerTop2 = StructGridTools::cylindricalToCartesian( cell.outerRadius, angle2, cell.topZ );

        // Create quad for curved surface (reverse winding for outward face)
        vertices.push_back( cvf::Vec3f( outerBot2 - offset ) );
        vertices.push_back( cvf::Vec3f( outerBot1 - offset ) );
        vertices.push_back( cvf::Vec3f( outerTop1 - offset ) );
        vertices.push_back( cvf::Vec3f( outerTop2 - offset ) );

        m_quadMapper->quadToCellIndexMap().push_back( cellIndex );
        m_quadMapper->quadToCellFaceMap().push_back( StructGridInterface::POS_I );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CylindricalGeometryGenerator::addCircumferentialFaces( const cvf::CylindricalCell& cell,
                                                            size_t                      cellIndex,
                                                            std::vector<Vec3f>&         vertices )
{
    cvf::Vec3d offset = m_grid->displayModelOffset();

    // Start angle face (NEG_J) - straight radial face, no radial subdivisions
    cvf::Vec3d startInner = StructGridTools::cylindricalToCartesian( cell.innerRadius, cell.startAngle, cell.bottomZ );
    cvf::Vec3d startOuter = StructGridTools::cylindricalToCartesian( cell.outerRadius, cell.startAngle, cell.bottomZ );
    cvf::Vec3d startInnerTop = StructGridTools::cylindricalToCartesian( cell.innerRadius, cell.startAngle, cell.topZ );
    cvf::Vec3d startOuterTop = StructGridTools::cylindricalToCartesian( cell.outerRadius, cell.startAngle, cell.topZ );

    vertices.push_back( cvf::Vec3f( startInner - offset ) );
    vertices.push_back( cvf::Vec3f( startOuter - offset ) );
    vertices.push_back( cvf::Vec3f( startOuterTop - offset ) );
    vertices.push_back( cvf::Vec3f( startInnerTop - offset ) );

    m_quadMapper->quadToCellIndexMap().push_back( cellIndex );
    m_quadMapper->quadToCellFaceMap().push_back( StructGridInterface::NEG_J );

    // End angle face (POS_J) - straight radial face, no radial subdivisions
    cvf::Vec3d endInner    = StructGridTools::cylindricalToCartesian( cell.innerRadius, cell.endAngle, cell.bottomZ );
    cvf::Vec3d endOuter    = StructGridTools::cylindricalToCartesian( cell.outerRadius, cell.endAngle, cell.bottomZ );
    cvf::Vec3d endInnerTop = StructGridTools::cylindricalToCartesian( cell.innerRadius, cell.endAngle, cell.topZ );
    cvf::Vec3d endOuterTop = StructGridTools::cylindricalToCartesian( cell.outerRadius, cell.endAngle, cell.topZ );

    vertices.push_back( cvf::Vec3f( endOuter - offset ) );
    vertices.push_back( cvf::Vec3f( endInner - offset ) );
    vertices.push_back( cvf::Vec3f( endInnerTop - offset ) );
    vertices.push_back( cvf::Vec3f( endOuterTop - offset ) );

    m_quadMapper->quadToCellIndexMap().push_back( cellIndex );
    m_quadMapper->quadToCellFaceMap().push_back( StructGridInterface::POS_J );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CylindricalGeometryGenerator::addTopBottomFaces( const cvf::CylindricalCell& cell,
                                                      size_t                      cellIndex,
                                                      std::vector<Vec3f>&         vertices )
{
    cvf::Vec3d offset = m_grid->displayModelOffset();

    // Create angular subdivisions only for smooth curved surfaces on top/bottom faces
    double angleRange = cell.endAngle - cell.startAngle;
    double angleStep  = angleRange / m_curveSubdivisions;

    // Bottom face (NEG_K) - angular subdivision only
    for ( int j = 0; j < m_curveSubdivisions; ++j )
    {
        double angle1 = cell.startAngle + j * angleStep;
        double angle2 = cell.startAngle + ( j + 1 ) * angleStep;

        // Create quad using only inner and outer radius
        cvf::Vec3d bottomInnerStart = StructGridTools::cylindricalToCartesian( cell.innerRadius, angle1, cell.bottomZ );
        cvf::Vec3d bottomOuterStart = StructGridTools::cylindricalToCartesian( cell.outerRadius, angle1, cell.bottomZ );
        cvf::Vec3d bottomInnerEnd   = StructGridTools::cylindricalToCartesian( cell.innerRadius, angle2, cell.bottomZ );
        cvf::Vec3d bottomOuterEnd   = StructGridTools::cylindricalToCartesian( cell.outerRadius, angle2, cell.bottomZ );

        vertices.push_back( cvf::Vec3f( bottomInnerStart - offset ) );
        vertices.push_back( cvf::Vec3f( bottomOuterStart - offset ) );
        vertices.push_back( cvf::Vec3f( bottomOuterEnd - offset ) );
        vertices.push_back( cvf::Vec3f( bottomInnerEnd - offset ) );

        m_quadMapper->quadToCellIndexMap().push_back( cellIndex );
        m_quadMapper->quadToCellFaceMap().push_back( StructGridInterface::NEG_K );
    }

    // Top face (POS_K) - angular subdivision only
    for ( int j = 0; j < m_curveSubdivisions; ++j )
    {
        double angle1 = cell.startAngle + j * angleStep;
        double angle2 = cell.startAngle + ( j + 1 ) * angleStep;

        // Create quad using only inner and outer radius (reverse winding for upward face)
        cvf::Vec3d topInnerStart = StructGridTools::cylindricalToCartesian( cell.innerRadius, angle1, cell.topZ );
        cvf::Vec3d topOuterStart = StructGridTools::cylindricalToCartesian( cell.outerRadius, angle1, cell.topZ );
        cvf::Vec3d topInnerEnd   = StructGridTools::cylindricalToCartesian( cell.innerRadius, angle2, cell.topZ );
        cvf::Vec3d topOuterEnd   = StructGridTools::cylindricalToCartesian( cell.outerRadius, angle2, cell.topZ );

        vertices.push_back( cvf::Vec3f( topInnerEnd - offset ) );
        vertices.push_back( cvf::Vec3f( topOuterEnd - offset ) );
        vertices.push_back( cvf::Vec3f( topOuterStart - offset ) );
        vertices.push_back( cvf::Vec3f( topInnerStart - offset ) );

        m_quadMapper->quadToCellIndexMap().push_back( cellIndex );
        m_quadMapper->quadToCellFaceMap().push_back( StructGridInterface::POS_K );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool CylindricalGeometryGenerator::isCellFaceVisible( size_t i, size_t j, size_t k, StructGridInterface::FaceType face ) const
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
ref<UIntArray> CylindricalGeometryGenerator::createCylindricalMeshLineIndices()
{
    CVF_ASSERT( m_vertices.notNull() );

    size_t numVertices = m_vertices->size();
    int    numQuads    = static_cast<int>( numVertices / 4 );
    CVF_ASSERT( numVertices % 4 == 0 );

    std::vector<uint> filteredIndices;
    filteredIndices.reserve( numQuads * 8 );

    for ( int i = 0; i < numQuads; i++ )
    {
        // Get the face type for this quad
        StructGridInterface::FaceType faceType = m_quadMapper->cellFace( i );

        // For top/bottom faces (NEG_K, POS_K), skip all internal subdivision lines
        if ( faceType == StructGridInterface::NEG_K || faceType == StructGridInterface::POS_K )
        {
            // Skip internal mesh lines on top/bottom faces - only perimeter will be visible
            // from circumferential faces
        }
        else if ( faceType == StructGridInterface::NEG_I || faceType == StructGridInterface::POS_I )
        {
            // For radial faces, only add the curved (horizontal) lines, skip vertical subdivisions
            filteredIndices.push_back( i * 4 + 0 );
            filteredIndices.push_back( i * 4 + 1 );
            filteredIndices.push_back( i * 4 + 2 );
            filteredIndices.push_back( i * 4 + 3 );
        }
        else
        {
            // For circumferential faces, add all edge lines (these are the corner lines)
            filteredIndices.push_back( i * 4 + 0 );
            filteredIndices.push_back( i * 4 + 1 );
            filteredIndices.push_back( i * 4 + 1 );
            filteredIndices.push_back( i * 4 + 2 );
            filteredIndices.push_back( i * 4 + 2 );
            filteredIndices.push_back( i * 4 + 3 );
            filteredIndices.push_back( i * 4 + 3 );
            filteredIndices.push_back( i * 4 + 0 );
        }
    }

    ref<UIntArray> indices = new UIntArray;
    indices->assign( filteredIndices );
    return indices;
}

} // namespace cvf
