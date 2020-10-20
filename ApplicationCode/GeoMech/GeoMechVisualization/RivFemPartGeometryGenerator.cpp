/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RivFemPartGeometryGenerator.h"

#include "RigFemPart.h"

#include "cvfArray.h"
#include "cvfDebugTimer.h"
#include "cvfDrawableGeo.h"
#include "cvfOutlineEdgeExtractor.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfScalarMapper.h"

#include <cmath>
#include <cstdlib>

using namespace cvf;

//==================================================================================================
///
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivFemPartGeometryGenerator::RivFemPartGeometryGenerator( const RigFemPart* part )
    : m_part( part )
{
    CVF_ASSERT( part );
    m_triangleMapper = new RivFemPartTriangleToElmMapper;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivFemPartGeometryGenerator::~RivFemPartGeometryGenerator()
{
}

//--------------------------------------------------------------------------------------------------
/// Generate surface drawable geo from the specified region
///
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> RivFemPartGeometryGenerator::generateSurface()
{
    computeArrays();

    CVF_ASSERT( m_quadVertices.notNull() );

    if ( m_quadVertices->size() == 0 ) return nullptr;

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setFromQuadVertexArray( m_quadVertices.p() );

    return geo;
}

//--------------------------------------------------------------------------------------------------
/// Generates simplified mesh as line drawing
/// Must call generateSurface first
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> RivFemPartGeometryGenerator::createMeshDrawable()
{
    if ( !( m_quadVertices.notNull() && m_quadVertices->size() != 0 ) ) return nullptr;

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray( m_quadVertices.p() );

    ref<UIntArray>               indices = lineIndicesFromQuadVertexArray( m_quadVertices.p() );
    ref<PrimitiveSetIndexedUInt> prim    = new PrimitiveSetIndexedUInt( PT_LINES );
    prim->setIndices( indices.p() );

    geo->addPrimitiveSet( prim.p() );
    return geo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> RivFemPartGeometryGenerator::createOutlineMeshDrawable( double creaseAngle )
{
    if ( !( m_quadVertices.notNull() && m_quadVertices->size() != 0 ) ) return nullptr;

    cvf::OutlineEdgeExtractor ee( creaseAngle, *m_quadVertices );

    ref<UIntArray> indices = lineIndicesFromQuadVertexArray( m_quadVertices.p() );
    ee.addPrimitives( 4, *indices );

    ref<cvf::UIntArray> lineIndices = ee.lineIndices();
    if ( lineIndices->size() == 0 )
    {
        return nullptr;
    }

    ref<PrimitiveSetIndexedUInt> prim = new PrimitiveSetIndexedUInt( PT_LINES );
    prim->setIndices( lineIndices.p() );

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray( m_quadVertices.p() );
    geo->addPrimitiveSet( prim.p() );

    return geo;
}

//--------------------------------------------------------------------------------------------------
///
///
///
///
//--------------------------------------------------------------------------------------------------
ref<UIntArray> RivFemPartGeometryGenerator::lineIndicesFromQuadVertexArray( const Vec3fArray* vertexArray )
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
void RivFemPartGeometryGenerator::computeArrays()
{
    std::vector<Vec3f> vertices;
    std::vector<int>&  trianglesToElements     = m_triangleMapper->triangleToElmIndexMap();
    std::vector<char>& trianglesToElementFaces = m_triangleMapper->triangleToElmFaceMap();

    m_quadVerticesToNodeIdx.clear();
    m_quadVerticesToGlobalElmNodeIdx.clear();
    m_quadVerticesToGlobalElmFaceNodeIdx.clear();
    m_quadVerticesToGlobalElmIdx.clear();
    trianglesToElements.clear();
    trianglesToElementFaces.clear();

    size_t estimatedQuadVxCount = m_part->elementCount() * 6 * 4;

    vertices.reserve( estimatedQuadVxCount );
    m_quadVerticesToNodeIdx.reserve( estimatedQuadVxCount );
    m_quadVerticesToGlobalElmNodeIdx.reserve( estimatedQuadVxCount );
    m_quadVerticesToGlobalElmIdx.reserve( estimatedQuadVxCount );
    trianglesToElements.reserve( estimatedQuadVxCount / 2 );
    trianglesToElementFaces.reserve( estimatedQuadVxCount / 2 );

    cvf::Vec3d                     displayOffset   = m_part->boundingBox().min();
    const std::vector<cvf::Vec3f>& nodeCoordinates = m_part->nodes().coordinates;

#pragma omp parallel for schedule( dynamic )
    for ( int elmIdx = 0; elmIdx < static_cast<int>( m_part->elementCount() ); elmIdx++ )
    {
        if ( m_elmVisibility.isNull() || ( *m_elmVisibility )[elmIdx] )
        {
            RigElementType eType     = m_part->elementType( elmIdx );
            int            faceCount = RigFemTypes::elementFaceCount( eType );

            const int* elmNodeIndices = m_part->connectivities( elmIdx );

            int elmNodFaceResIdxElmStart = elmIdx * 24; // HACK should get from part

            for ( int lfIdx = 0; lfIdx < faceCount; ++lfIdx )
            {
                int elmNeighbor = m_part->elementNeighbor( elmIdx, lfIdx );

                if ( elmNeighbor != -1 && ( m_elmVisibility.isNull() || ( *m_elmVisibility )[elmNeighbor] ) )
                {
                    continue; // Invisible face
                }

                int        faceNodeCount = 0;
                const int* localElmNodeIndicesForFace =
                    RigFemTypes::localElmNodeIndicesForFace( eType, lfIdx, &faceNodeCount );
                if ( faceNodeCount == 4 )
                {
                    cvf::Vec3f quadVxs0( cvf::Vec3d( nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[0]]] ) -
                                         displayOffset );
                    cvf::Vec3f quadVxs1( cvf::Vec3d( nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[1]]] ) -
                                         displayOffset );
                    cvf::Vec3f quadVxs2( cvf::Vec3d( nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[2]]] ) -
                                         displayOffset );
                    cvf::Vec3f quadVxs3( cvf::Vec3d( nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[3]]] ) -
                                         displayOffset );

                    int qNodeIdx[4];
                    qNodeIdx[0] = elmNodeIndices[localElmNodeIndicesForFace[0]];
                    qNodeIdx[1] = elmNodeIndices[localElmNodeIndicesForFace[1]];
                    qNodeIdx[2] = elmNodeIndices[localElmNodeIndicesForFace[2]];
                    qNodeIdx[3] = elmNodeIndices[localElmNodeIndicesForFace[3]];

                    size_t qElmNodeResIdx[4];
                    qElmNodeResIdx[0] = m_part->elementNodeResultIdx( elmIdx, localElmNodeIndicesForFace[0] );
                    qElmNodeResIdx[1] = m_part->elementNodeResultIdx( elmIdx, localElmNodeIndicesForFace[1] );
                    qElmNodeResIdx[2] = m_part->elementNodeResultIdx( elmIdx, localElmNodeIndicesForFace[2] );
                    qElmNodeResIdx[3] = m_part->elementNodeResultIdx( elmIdx, localElmNodeIndicesForFace[3] );

#pragma omp critical( critical_section_RivFemPartGeometryGenerator_computeArrays )
                    {
                        vertices.push_back( quadVxs0 );
                        vertices.push_back( quadVxs1 );
                        vertices.push_back( quadVxs2 );
                        vertices.push_back( quadVxs3 );

                        m_quadVerticesToNodeIdx.push_back( qNodeIdx[0] );
                        m_quadVerticesToNodeIdx.push_back( qNodeIdx[1] );
                        m_quadVerticesToNodeIdx.push_back( qNodeIdx[2] );
                        m_quadVerticesToNodeIdx.push_back( qNodeIdx[3] );

                        m_quadVerticesToGlobalElmNodeIdx.push_back( qElmNodeResIdx[0] );
                        m_quadVerticesToGlobalElmNodeIdx.push_back( qElmNodeResIdx[1] );
                        m_quadVerticesToGlobalElmNodeIdx.push_back( qElmNodeResIdx[2] );
                        m_quadVerticesToGlobalElmNodeIdx.push_back( qElmNodeResIdx[3] );

                        int elmNodFaceResIdxFaceStart = elmNodFaceResIdxElmStart + lfIdx * 4; // HACK

                        m_quadVerticesToGlobalElmFaceNodeIdx.push_back( elmNodFaceResIdxFaceStart + 0 );
                        m_quadVerticesToGlobalElmFaceNodeIdx.push_back( elmNodFaceResIdxFaceStart + 1 );
                        m_quadVerticesToGlobalElmFaceNodeIdx.push_back( elmNodFaceResIdxFaceStart + 2 );
                        m_quadVerticesToGlobalElmFaceNodeIdx.push_back( elmNodFaceResIdxFaceStart + 3 );

                        m_quadVerticesToGlobalElmIdx.push_back( elmIdx );
                        m_quadVerticesToGlobalElmIdx.push_back( elmIdx );
                        m_quadVerticesToGlobalElmIdx.push_back( elmIdx );
                        m_quadVerticesToGlobalElmIdx.push_back( elmIdx );

                        trianglesToElements.push_back( elmIdx );
                        trianglesToElements.push_back( elmIdx );
                        trianglesToElementFaces.push_back( lfIdx );
                        trianglesToElementFaces.push_back( lfIdx );
                    }
                }
                else
                {
                    // Handle triangles and 6 node and 8 node faces
                }
            }
        }
    }

    m_quadVertices = new cvf::Vec3fArray;
    m_quadVertices->assign( vertices );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivFemPartGeometryGenerator::setElementVisibility( const cvf::UByteArray* cellVisibility )
{
    m_elmVisibility = cellVisibility;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo>
    RivFemPartGeometryGenerator::createMeshDrawableFromSingleElement( const RigFemPart* part,
                                                                      size_t            elmIdx,
                                                                      const cvf::Vec3d& displayModelOffset )
{
    cvf::ref<cvf::Vec3fArray> quadVertices;

    {
        std::vector<Vec3f> vertices;

        const std::vector<cvf::Vec3f>& nodeCoordinates = part->nodes().coordinates;

        RigElementType eType     = part->elementType( elmIdx );
        int            faceCount = RigFemTypes::elementFaceCount( eType );

        const int* elmNodeIndices = part->connectivities( elmIdx );

        // cvf::Vec3d displayOffset = part->boundingBox().min();

        for ( int lfIdx = 0; lfIdx < faceCount; ++lfIdx )
        {
            int        faceNodeCount              = 0;
            const int* localElmNodeIndicesForFace = RigFemTypes::localElmNodeIndicesForFace( eType, lfIdx, &faceNodeCount );
            if ( faceNodeCount == 4 )
            {
                vertices.push_back( cvf::Vec3f(
                    cvf::Vec3d( nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[0]]] ) - displayModelOffset ) );
                vertices.push_back( cvf::Vec3f(
                    cvf::Vec3d( nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[1]]] ) - displayModelOffset ) );
                vertices.push_back( cvf::Vec3f(
                    cvf::Vec3d( nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[2]]] ) - displayModelOffset ) );
                vertices.push_back( cvf::Vec3f(
                    cvf::Vec3d( nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[3]]] ) - displayModelOffset ) );
            }
            else
            {
                // Handle triangles and 6 node and 8 node faces
            }
        }

        quadVertices = new cvf::Vec3fArray;
        quadVertices->assign( vertices );
    }

    if ( !( quadVertices.notNull() && quadVertices->size() != 0 ) ) return nullptr;

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray( quadVertices.p() );

    ref<UIntArray>               indices = lineIndicesFromQuadVertexArray( quadVertices.p() );
    ref<PrimitiveSetIndexedUInt> prim    = new PrimitiveSetIndexedUInt( PT_LINES );
    prim->setIndices( indices.p() );

    geo->addPrimitiveSet( prim.p() );
    return geo;
}
