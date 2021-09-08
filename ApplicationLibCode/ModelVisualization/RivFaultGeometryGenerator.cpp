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

#include "RivFaultGeometryGenerator.h"

#include <cmath>

#include "cvfDrawableGeo.h"
#include "cvfOutlineEdgeExtractor.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfStructGridGeometryGenerator.h"

#include "cvfScalarMapper.h"

#include "RigFault.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivFaultGeometryGenerator::RivFaultGeometryGenerator( const cvf::StructGridInterface* grid,
                                                      const RigFault*                 fault,
                                                      bool                            computeNativeFaultFaces )
    : m_grid( grid )
    , m_fault( fault )
    , m_computeNativeFaultFaces( computeNativeFaultFaces )
{
    m_quadMapper     = new cvf::StructGridQuadToCellFaceMapper;
    m_triangleMapper = new cvf::StuctGridTriangleToCellFaceMapper( m_quadMapper.p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivFaultGeometryGenerator::~RivFaultGeometryGenerator()
{
}

//--------------------------------------------------------------------------------------------------
/// Generate surface drawable geo from the specified region
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivFaultGeometryGenerator::generateSurface()
{
    computeArrays();

    CVF_ASSERT( m_vertices.notNull() );

    if ( m_vertices->size() == 0 ) return nullptr;

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->setFromQuadVertexArray( m_vertices.p() );

    return geo;
}

//--------------------------------------------------------------------------------------------------
/// Generates simplified mesh as line drawing
/// Must call generateSurface first
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivFaultGeometryGenerator::createMeshDrawable()
{
    if ( !( m_vertices.notNull() && m_vertices->size() != 0 ) ) return nullptr;

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->setVertexArray( m_vertices.p() );

    cvf::ref<cvf::UIntArray> indices = cvf::StructGridGeometryGenerator::lineIndicesFromQuadVertexArray( m_vertices.p() );

    cvf::ref<cvf::PrimitiveSetIndexedUInt> prim = new cvf::PrimitiveSetIndexedUInt( cvf::PT_LINES );
    prim->setIndices( indices.p() );

    geo->addPrimitiveSet( prim.p() );
    return geo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivFaultGeometryGenerator::createOutlineMeshDrawable( double creaseAngle )
{
    if ( !( m_vertices.notNull() && m_vertices->size() != 0 ) ) return nullptr;

    cvf::OutlineEdgeExtractor ee( creaseAngle, *m_vertices );

    cvf::ref<cvf::UIntArray> indices = cvf::StructGridGeometryGenerator::lineIndicesFromQuadVertexArray( m_vertices.p() );
    ee.addPrimitives( 4, *indices );

    cvf::ref<cvf::UIntArray> lineIndices = ee.lineIndices();
    if ( lineIndices->size() == 0 )
    {
        return nullptr;
    }

    cvf::ref<cvf::PrimitiveSetIndexedUInt> prim = new cvf::PrimitiveSetIndexedUInt( cvf::PT_LINES );
    prim->setIndices( lineIndices.p() );

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->setVertexArray( m_vertices.p() );
    geo->addPrimitiveSet( prim.p() );

    return geo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivFaultGeometryGenerator::computeArrays()
{
    std::vector<cvf::Vec3f> vertices;
    m_quadMapper->quadToCellIndexMap().clear();
    m_quadMapper->quadToCellFaceMap().clear();

    cvf::Vec3d offset = m_grid->displayModelOffset();

    const std::vector<RigFault::FaultFace>& faultFaces = m_fault->faultFaces();

#pragma omp parallel for
    for ( int fIdx = 0; fIdx < static_cast<int>( faultFaces.size() ); fIdx++ )
    {
        size_t                             cellIndex = faultFaces[fIdx].m_nativeReservoirCellIndex;
        cvf::StructGridInterface::FaceType face      = faultFaces[fIdx].m_nativeFace;

        if ( cellIndex >= m_cellVisibility->size() ) continue;

        if ( !m_computeNativeFaultFaces )
        {
            cellIndex = faultFaces[fIdx].m_oppositeReservoirCellIndex;
            face      = cvf::StructGridInterface::oppositeFace( faultFaces[fIdx].m_nativeFace );
        }

        if ( !( *m_cellVisibility )[cellIndex] ) continue;

        cvf::Vec3d cornerVerts[8];
        m_grid->cellCornerVertices( cellIndex, cornerVerts );

        cvf::ubyte faceConn[4];
        m_grid->cellFaceVertexIndices( face, faceConn );

        // Critical section to avoid two threads accessing the arrays at the same time.
#pragma omp critical( critical_section_RivFaultGeometryGenerator_computeArrays )
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

    m_vertices = new cvf::Vec3fArray;
    m_vertices->assign( vertices );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivFaultGeometryGenerator::setCellVisibility( const cvf::UByteArray* cellVisibility )
{
    m_cellVisibility = cellVisibility;
}
