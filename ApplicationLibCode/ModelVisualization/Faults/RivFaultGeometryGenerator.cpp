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

#include "RiaOpenMPTools.h"

#include "RigFault.h"
#include "RigNNCData.h"
#include "RigNncConnection.h"

#include "cvfDrawableGeo.h"
#include "cvfOutlineEdgeExtractor.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfStructGridGeometryGenerator.h"
#include "cvfStructGridTools.h"

#include "cvfScalarMapper.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivFaultGeometryGenerator::RivFaultGeometryGenerator( const cvf::StructGridInterface* grid,
                                                      const RigFault*                 fault,
                                                      RigNNCData*                     nncData,
                                                      bool                            computeNativeFaultFaces )
    : m_grid( grid )
    , m_fault( fault )
    , m_computeNativeFaultFaces( computeNativeFaultFaces )
    , m_nncData( nncData )
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
cvf::ref<cvf::DrawableGeo> RivFaultGeometryGenerator::generateSurface( bool onlyShowFacesWithDefinedNeighbors )
{
    computeArrays( onlyShowFacesWithDefinedNeighbors );

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
    if ( !m_vertices.notNull() || m_vertices->size() == 0 ) return nullptr;

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->setVertexArray( m_vertices.p() );

    cvf::ref<cvf::UIntArray> indices = cvf::StructGridTools::lineIndicesFromQuadVertexArray( m_vertices.p() );

    cvf::ref<cvf::PrimitiveSetIndexedUInt> prim = new cvf::PrimitiveSetIndexedUInt( cvf::PT_LINES );
    prim->setIndices( indices.p() );

    geo->addPrimitiveSet( prim.p() );
    return geo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivFaultGeometryGenerator::hasConnection( size_t                             cellIdx,
                                               cvf::StructGridInterface::FaceType face,
                                               const RigConnectionContainer&      conns,
                                               const std::vector<size_t>&         nncConnectionIndices )
{
    cvf::StructGridInterface::FaceType oppositeFace = cvf::StructGridInterface::oppositeFace( face );

    for ( auto i : nncConnectionIndices )
    {
        if ( i >= conns.size() ) continue;

        const auto& r = conns[i];

        if ( ( r.c1GlobIdx() == cellIdx ) && ( r.face() == face ) && r.hasCommonArea() ) return true;

        if ( ( r.c2GlobIdx() == cellIdx ) && ( r.face() == oppositeFace ) && r.hasCommonArea() ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivFaultGeometryGenerator::computeArrays( bool onlyShowFacesWithDefinedNeighbors )
{
    std::vector<cvf::Vec3f> vertices;
    m_quadMapper->quadToCellIndexMap().clear();
    m_quadMapper->quadToCellFaceMap().clear();

    cvf::Vec3d offset = m_grid->displayModelOffset();

    if ( onlyShowFacesWithDefinedNeighbors )
    {
        // Make sure the connection polygon is computed, as this is used as criteria for visibility
        m_nncData->ensureAllConnectionDataIsProcessed();
    }

    auto  connIndices = m_fault->connectionIndices();
    auto& connections = m_nncData->availableConnections();

    const std::vector<RigFault::FaultFace>& faultFaces = m_fault->faultFaces();

    int numberOfThreads = RiaOpenMPTools::availableThreadCount();

    std::vector<std::vector<cvf::Vec3f>>                         threadVertices( numberOfThreads );
    std::vector<std::vector<size_t>>                             threadCellIndices( numberOfThreads );
    std::vector<std::vector<cvf::StructGridInterface::FaceType>> threadFaceTypes( numberOfThreads );

#pragma omp parallel
    {
        int myThread = RiaOpenMPTools::currentThreadIndex();

        cvf::ubyte faceConn[4];

        // NB! We are inside a parallel section, do not use "parallel for" here
#pragma omp for
        for ( int fIdx = 0; fIdx < static_cast<int>( faultFaces.size() ); fIdx++ )
        {
            size_t                             cellIndex = faultFaces[fIdx].m_nativeReservoirCellIndex;
            cvf::StructGridInterface::FaceType face      = faultFaces[fIdx].m_nativeFace;

            if ( !m_computeNativeFaultFaces )
            {
                cellIndex = faultFaces[fIdx].m_oppositeReservoirCellIndex;
                face      = cvf::StructGridInterface::oppositeFace( face );
            }

            if ( cellIndex >= m_cellVisibility->size() ) continue;

            if ( !( *m_cellVisibility )[cellIndex] ) continue;

            if ( onlyShowFacesWithDefinedNeighbors && !hasConnection( cellIndex, face, connections, connIndices ) ) continue;

            std::array<cvf::Vec3d, 8> cornerVerts = m_grid->cellCornerVertices( cellIndex );
            cvf::StructGridInterface::cellFaceVertexIndices( face, faceConn );

            for ( int n = 0; n < 4; n++ )
            {
                threadVertices[myThread].emplace_back( cvf::Vec3f( cornerVerts[faceConn[n]] - offset ) );
            }

            // Keep track of the source cell index per quad
            threadCellIndices[myThread].emplace_back( cellIndex );
            threadFaceTypes[myThread].emplace_back( face );
        }
    }

    for ( int threadIndex = 0; threadIndex < numberOfThreads; threadIndex++ )
    {
        vertices.insert( vertices.end(), threadVertices[threadIndex].begin(), threadVertices[threadIndex].end() );

        m_quadMapper->quadToCellIndexMap().insert( m_quadMapper->quadToCellIndexMap().end(),
                                                   threadCellIndices[threadIndex].begin(),
                                                   threadCellIndices[threadIndex].end() );

        m_quadMapper->quadToCellFaceMap().insert( m_quadMapper->quadToCellFaceMap().end(),
                                                  threadFaceTypes[threadIndex].begin(),
                                                  threadFaceTypes[threadIndex].end() );
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
