/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RivIjkIntersectionGeometryGenerator.h"

#include "RivIntersectionHexGridInterface.h"

#include "RigMainGrid.h"

#include "RimIjkIntersection.h"

#include "cvfDrawableGeo.h"
#include "cvfPrimitiveSetDirect.h"
#include "cvfStructGrid.h"

#include <algorithm>
#include <array>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivIjkIntersectionGeometryGenerator::RivIjkIntersectionGeometryGenerator( RimIjkIntersection*                    intersection,
                                                                          const RivIntersectionHexGridInterface* grid,
                                                                          const RigMainGrid*                     mainGrid )
    : m_hexGrid( grid )
    , m_mainGrid( mainGrid )
    , m_intersectionDefinition( intersection )
{
    m_triangleVxes       = new cvf::Vec3fArray;
    m_cellBorderLineVxes = new cvf::Vec3fArray;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivIjkIntersectionGeometryGenerator::~RivIjkIntersectionGeometryGenerator()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivIjkIntersectionGeometryGenerator::isAnyGeometryPresent() const
{
    return m_triangleVxes->size() != 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivIjkIntersectionGeometryGenerator::generateSurface( cvf::UByteArray* visibleCells )
{
    calculateArrays( visibleCells );

    CVF_ASSERT( m_triangleVxes.notNull() );

    if ( m_triangleVxes->size() == 0 ) return nullptr;

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->setFromTriangleVertexArray( m_triangleVxes.p() );

    return geo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivIjkIntersectionGeometryGenerator::createMeshDrawable()
{
    if ( !m_cellBorderLineVxes.notNull() || m_cellBorderLineVxes->size() == 0 ) return nullptr;

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->setVertexArray( m_cellBorderLineVxes.p() );

    cvf::ref<cvf::PrimitiveSetDirect> prim = new cvf::PrimitiveSetDirect( cvf::PT_LINES );
    prim->setIndexCount( m_cellBorderLineVxes->size() );

    geo->addPrimitiveSet( prim.p() );
    return geo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RivIjkIntersectionGeometryGenerator::triangleToCellIndex() const
{
    CVF_ASSERT( m_triangleVxes->size() );
    return m_triangleToCellIdxMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RivIntersectionVertexWeights>& RivIjkIntersectionGeometryGenerator::triangleVxToCellCornerInterpolationWeights() const
{
    CVF_ASSERT( m_triangleVxes->size() );
    return m_triVxToCellCornerWeights;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::Vec3fArray* RivIjkIntersectionGeometryGenerator::triangleVxes() const
{
    CVF_ASSERT( m_triangleVxes->size() );

    return m_triangleVxes.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIjkIntersection* RivIjkIntersectionGeometryGenerator::intersection() const
{
    return m_intersectionDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivIjkIntersectionGeometryGenerator::calculateArrays( cvf::UByteArray* visibleCells )
{
    if ( m_triangleVxes->size() ) return;
    if ( m_mainGrid.isNull() || m_hexGrid.isNull() ) return;

    using FaceType = cvf::StructGridInterface::FaceType;

    const int ni = static_cast<int>( m_mainGrid->cellCountI() );
    const int nj = static_cast<int>( m_mainGrid->cellCountJ() );
    const int nk = static_cast<int>( m_mainGrid->cellCountK() );
    if ( ni == 0 || nj == 0 || nk == 0 ) return;

    // Determine which cell face the surface follows and the i/j/k loop bounds
    RimIjkIntersection::GridAxis axis = m_intersectionDefinition->axis();
    bool                         neg  = m_intersectionDefinition->useNegativeFace();

    int iMin, iMax, jMin, jMax, kMin, kMax;
    m_intersectionDefinition->ijkRange( iMin, iMax, jMin, jMax, kMin, kMax );
    int fixed = m_intersectionDefinition->fixedIndex();

    FaceType face = FaceType::POS_K;
    switch ( axis )
    {
        case RimIjkIntersection::GridAxis::AXIS_I:
            face  = neg ? FaceType::NEG_I : FaceType::POS_I;
            fixed = std::clamp( fixed, 0, ni - 1 );
            iMin = iMax = fixed;
            jMin        = std::clamp( jMin, 0, nj - 1 );
            jMax        = std::clamp( jMax, 0, nj - 1 );
            kMin        = std::clamp( kMin, 0, nk - 1 );
            kMax        = std::clamp( kMax, 0, nk - 1 );
            break;
        case RimIjkIntersection::GridAxis::AXIS_J:
            face  = neg ? FaceType::NEG_J : FaceType::POS_J;
            fixed = std::clamp( fixed, 0, nj - 1 );
            jMin = jMax = fixed;
            iMin        = std::clamp( iMin, 0, ni - 1 );
            iMax        = std::clamp( iMax, 0, ni - 1 );
            kMin        = std::clamp( kMin, 0, nk - 1 );
            kMax        = std::clamp( kMax, 0, nk - 1 );
            break;
        case RimIjkIntersection::GridAxis::AXIS_K:
            face  = neg ? FaceType::NEG_K : FaceType::POS_K;
            fixed = std::clamp( fixed, 0, nk - 1 );
            kMin = kMax = fixed;
            iMin        = std::clamp( iMin, 0, ni - 1 );
            iMax        = std::clamp( iMax, 0, ni - 1 );
            jMin        = std::clamp( jMin, 0, nj - 1 );
            jMax        = std::clamp( jMax, 0, nj - 1 );
            break;
    }

    cvf::ubyte faceVtxIdx[4];
    cvf::StructGridInterface::cellFaceVertexIndices( face, faceVtxIdx );

    cvf::Vec3d displayOffset = m_hexGrid->displayOffset();

    std::vector<cvf::Vec3f> triangleVertices;
    std::vector<cvf::Vec3f> cellBorderLineVxes;

    for ( int k = kMin; k <= kMax; ++k )
    {
        for ( int j = jMin; j <= jMax; ++j )
        {
            for ( int i = iMin; i <= iMax; ++i )
            {
                size_t globalCellIdx = m_mainGrid->cellIndexFromIJK( i, j, k );

                if ( ( visibleCells != nullptr ) && ( ( *visibleCells )[globalCellIdx] == 0 ) ) continue;
                if ( !m_hexGrid->useCell( globalCellIdx ) ) continue;

                const std::array<cvf::Vec3d, 8> cellCorners   = m_hexGrid->cellCornerVertices( globalCellIdx );
                const std::array<size_t, 8>     cornerIndices = m_hexGrid->cellCornerIndices( globalCellIdx );

                cvf::Vec3f faceVx[4];
                for ( int n = 0; n < 4; ++n )
                {
                    faceVx[n] = cvf::Vec3f( cellCorners[faceVtxIdx[n]] - displayOffset );
                }

                // Two triangles: (0, 1, 2) and (0, 2, 3)
                const int triVtxOrder[6] = { 0, 1, 2, 0, 2, 3 };
                for ( int n = 0; n < 6; ++n )
                {
                    int corner = triVtxOrder[n];
                    triangleVertices.push_back( faceVx[corner] );

                    size_t nodeId = cornerIndices[faceVtxIdx[corner]];
                    m_triVxToCellCornerWeights.push_back( RivIntersectionVertexWeights( nodeId, nodeId, 0.0 ) );
                }

                m_triangleToCellIdxMap.push_back( globalCellIdx );
                m_triangleToCellIdxMap.push_back( globalCellIdx );

                // Mesh border lines: the four face edges
                for ( int n = 0; n < 4; ++n )
                {
                    cellBorderLineVxes.push_back( faceVx[n] );
                    cellBorderLineVxes.push_back( faceVx[( n + 1 ) % 4] );
                }
            }
        }
    }

    m_cellBorderLineVxes->assign( cellBorderLineVxes );
    m_triangleVxes->assign( triangleVertices );
}
