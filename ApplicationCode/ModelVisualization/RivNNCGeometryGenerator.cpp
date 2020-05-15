/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RivNNCGeometryGenerator.h"

#include <cmath>

#include "RigCell.h"
#include "RigMainGrid.h"
#include "RigNNCData.h"
#include "cvfDrawableGeo.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfScalarMapper.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivNNCGeometryGenerator::RivNNCGeometryGenerator( bool                      includeAllan,
                                                  RigNNCData*               nncData,
                                                  const cvf::Vec3d&         offset,
                                                  const cvf::Array<size_t>* nncIndexes )
    : m_includeAllanDiagramGeometry( includeAllan )
    , m_nncData( nncData )
    , m_nncIndexes( nncIndexes )
    , m_offset( offset )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivNNCGeometryGenerator::~RivNNCGeometryGenerator()
{
}

//--------------------------------------------------------------------------------------------------
/// Generate surface drawable geo from the specified region
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivNNCGeometryGenerator::generateSurface()
{
    computeArrays();

    CVF_ASSERT( m_vertices.notNull() );

    if ( m_vertices->size() == 0 ) return nullptr;

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->setFromTriangleVertexArray( m_vertices.p() );

    return geo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivNNCGeometryGenerator::computeArrays()
{
    std::vector<cvf::Vec3f> vertices;
    std::vector<size_t>     triangleToNNC;

    const cvf::Vec3f offset( m_offset );
    long long        numConnections =
        static_cast<long long>( m_nncIndexes.isNull() ? m_nncData->connections().size() : m_nncIndexes->size() );

    bool                  isVisibilityCalcActive = m_cellVisibility.notNull() && m_grid.notNull();
    std::vector<RigCell>* allCells               = nullptr;
    if ( isVisibilityCalcActive )
    {
        allCells = &( m_grid->mainGrid()->globalCellArray() );
    }

#pragma omp parallel for ordered
    for ( long long nIdx = 0; nIdx < numConnections; ++nIdx )
    {
        size_t conIdx = m_nncIndexes.isNull() ? nIdx : ( *m_nncIndexes )[nIdx];

        if ( !m_includeAllanDiagramGeometry && conIdx >= m_nncData->nativeConnectionCount() )
        {
            continue;
        }

        const RigConnection& conn = m_nncData->connections()[conIdx];

        if ( conn.polygon().size() )
        {
            bool isVisible = true;
            if ( isVisibilityCalcActive )
            {
                bool cell1Visible = false;
                bool cell2Visible = false;

                if ( ( *allCells )[conn.c1GlobIdx()].hostGrid() == m_grid.p() )
                {
                    size_t cell1GridLocalIdx = ( *allCells )[conn.c1GlobIdx()].gridLocalCellIndex();
                    cell1Visible             = ( *m_cellVisibility )[cell1GridLocalIdx];
                }

                if ( ( *allCells )[conn.c2GlobIdx()].hostGrid() == m_grid.p() )
                {
                    size_t cell2GridLocalIdx = ( *allCells )[conn.c2GlobIdx()].gridLocalCellIndex();
                    cell2Visible             = ( *m_cellVisibility )[cell2GridLocalIdx];
                }

                isVisible = cell1Visible || cell2Visible;
            }

            if ( isVisible )
            {
                cvf::Vec3f vx1 = conn.polygon()[0] - offset;
                cvf::Vec3f vx2;
                cvf::Vec3f vx3 = conn.polygon()[1] - offset;

                for ( size_t vxIdx = 2; vxIdx < conn.polygon().size(); ++vxIdx )
                {
                    vx2 = vx3;
                    vx3 = conn.polygon()[vxIdx] - offset;
#pragma omp critical( critical_section_RivNNCGeometryGenerator_computeArrays )
                    {
                        vertices.push_back( vx1 );
                        vertices.push_back( vx2 );
                        vertices.push_back( vx3 );
                        triangleToNNC.push_back( conIdx );
                    }
                }
            }
        }
    }

    m_vertices = new cvf::Vec3fArray;
    m_vertices->assign( vertices );
    m_triangleIndexToNNCIndex = new cvf::Array<size_t>;
    m_triangleIndexToNNCIndex->assign( triangleToNNC );
}

//--------------------------------------------------------------------------------------------------
/// Calculates the texture coordinates in a "nearly" one dimensional texture.
/// Undefined values are coded with a y-texture coordinate value of 1.0 instead of the normal 0.5
//--------------------------------------------------------------------------------------------------
void RivNNCGeometryGenerator::textureCoordinates( cvf::Vec2fArray*               textureCoords,
                                                  const cvf::ScalarMapper*       mapper,
                                                  RiaDefines::ResultCatType      resultType,
                                                  const RigEclipseResultAddress& resVarAddr,
                                                  size_t                         nativeTimeStepIndex ) const
{
    size_t numVertices = m_vertices->size();

    textureCoords->resize( numVertices );
    cvf::Vec2f*                rawPtr        = textureCoords->ptr();
    const std::vector<double>* nncResultVals = nullptr;
    if ( resultType == RiaDefines::ResultCatType::STATIC_NATIVE ||
         resultType == RiaDefines::ResultCatType::FORMATION_NAMES ||
         resultType == RiaDefines::ResultCatType::ALLAN_DIAGRAMS )
    {
        nncResultVals = m_nncData->staticConnectionScalarResult( resVarAddr );
    }
    else if ( resultType == RiaDefines::ResultCatType::DYNAMIC_NATIVE )
    {
        nncResultVals = m_nncData->dynamicConnectionScalarResult( resVarAddr, nativeTimeStepIndex );
    }
    else if ( resultType == RiaDefines::ResultCatType::GENERATED )
    {
        nncResultVals = m_nncData->generatedConnectionScalarResult( resVarAddr, nativeTimeStepIndex );
    }

    if ( !nncResultVals || nncResultVals->size() == 0 )
    {
        textureCoords->setAll( cvf::Vec2f( 0.0f, 1.0f ) );
        return;
    }

#pragma omp parallel for
    for ( int tIdx = 0; tIdx < static_cast<int>( m_triangleIndexToNNCIndex->size() ); tIdx++ )
    {
        double cellScalarValue = HUGE_VAL;
        size_t resultIndex     = ( *m_triangleIndexToNNCIndex )[tIdx];

        // The nnc connections can have more connections than reported from Eclipse, clamp the result index to Eclipse
        // Results

        if ( resultIndex < nncResultVals->size() )
        {
            cellScalarValue = ( *nncResultVals )[resultIndex];
        }

        cvf::Vec2f texCoord = mapper->mapToTextureCoord( cellScalarValue );
        if ( cellScalarValue == HUGE_VAL || cellScalarValue != cellScalarValue ) // a != a is true for NAN's
        {
            texCoord[1] = 1.0f;
        }

        size_t j;
        for ( j = 0; j < 3; j++ )
        {
            rawPtr[tIdx * 3 + j] = texCoord;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivNNCGeometryGenerator::setCellVisibility( const cvf::UByteArray* cellVisibility, const RigGridBase* grid )
{
    m_cellVisibility = cellVisibility;
    m_grid           = grid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Array<size_t>> RivNNCGeometryGenerator::triangleToNNCIndex() const
{
    return m_triangleIndexToNNCIndex;
}
