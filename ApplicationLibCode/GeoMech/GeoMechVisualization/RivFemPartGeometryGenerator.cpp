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
#include "cvfDrawableGeo.h"
#include "cvfOutlineEdgeExtractor.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfScalarMapper.h"
#include "cvfStructGridGeometryGenerator.h"

#include "cafAssert.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <type_traits>

using namespace cvf;

//==================================================================================================
///
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivFemPartGeometryGenerator::RivFemPartGeometryGenerator( const RigFemPart* part, cvf::Vec3d displayOffset )
    : m_part( part )
    , m_displayOffset( displayOffset )
{
    CAF_ASSERT( part );
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
ref<DrawableGeo> RivFemPartGeometryGenerator::generateSurface( const std::vector<cvf::Vec3f>& nodeCoordinates )
{
    computeArrays( nodeCoordinates );

    CAF_ASSERT( m_quadVertices.notNull() );

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
    if ( !m_quadVertices.notNull() || m_quadVertices->size() == 0 ) return nullptr;

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray( m_quadVertices.p() );

    ref<UIntArray> indices = cvf::StructGridGeometryGenerator::lineIndicesFromQuadVertexArray( m_quadVertices.p() );

    ref<PrimitiveSetIndexedUInt> prim = new PrimitiveSetIndexedUInt( PT_LINES );
    prim->setIndices( indices.p() );

    geo->addPrimitiveSet( prim.p() );
    return geo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> RivFemPartGeometryGenerator::createOutlineMeshDrawable( double creaseAngle )
{
    if ( !m_quadVertices.notNull() || m_quadVertices->size() == 0 ) return nullptr;

    cvf::OutlineEdgeExtractor ee( creaseAngle, *m_quadVertices );

    ref<UIntArray> indices = cvf::StructGridGeometryGenerator::lineIndicesFromQuadVertexArray( m_quadVertices.p() );
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
//--------------------------------------------------------------------------------------------------
void RivFemPartGeometryGenerator::computeArrays( const std::vector<cvf::Vec3f>& nodeCoordinates )
{
    std::vector<int>&  trianglesToElements     = m_triangleMapper->triangleToElmIndexMap();
    std::vector<char>& trianglesToElementFaces = m_triangleMapper->triangleToElmFaceMap();

    m_quadVerticesToNodeIdx.clear();
    m_quadVerticesToGlobalElmNodeIdx.clear();
    m_quadVerticesToGlobalElmFaceNodeIdx.clear();
    m_quadVerticesToGlobalElmIdx.clear();
    trianglesToElements.clear();
    trianglesToElementFaces.clear();

    struct QuadArrays
    {
        std::vector<Vec3f>  vertices;
        std::vector<size_t> quadVerticesToNodeIdx;
        std::vector<size_t> quadVerticesToGlobalElmNodeIdx;
        std::vector<size_t> quadVerticesToGlobalElmFaceNodeIdx;
        std::vector<size_t> quadVerticesToGlobalElmIdx;
        std::vector<int>    trianglesToElements;
        std::vector<char>   trianglesToElementFaces;
    };

    // Collect the quads per block of elements and concatenate the blocks in element order afterwards, so the quad
    // order is independent of thread scheduling. A varying quad order gives nondeterministic rendering of
    // overlapping transparent faces.
    const int               elementCount     = static_cast<int>( m_part->elementCount() );
    const int               elementBlockSize = 4096;
    const int               blockCount       = ( elementCount + elementBlockSize - 1 ) / elementBlockSize;
    std::vector<QuadArrays> blockArrays( blockCount );

#pragma omp parallel for schedule( dynamic )
    for ( int blockIdx = 0; blockIdx < blockCount; blockIdx++ )
    {
        QuadArrays& arrays = blockArrays[blockIdx];

        const int blockEnd = std::min( elementCount, ( blockIdx + 1 ) * elementBlockSize );
        for ( int elmIdx = blockIdx * elementBlockSize; elmIdx < blockEnd; elmIdx++ )
        {
            if ( m_elmVisibility.notNull() && !( *m_elmVisibility )[elmIdx] ) continue;

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

                int        faceNodeCount              = 0;
                const int* localElmNodeIndicesForFace = RigFemTypes::localElmNodeIndicesForFace( eType, lfIdx, &faceNodeCount );
                if ( faceNodeCount == 4 )
                {
                    int elmNodFaceResIdxFaceStart = elmNodFaceResIdxElmStart + lfIdx * 4; // HACK

                    for ( int n = 0; n < 4; n++ )
                    {
                        const int nodeIdx = elmNodeIndices[localElmNodeIndicesForFace[n]];

                        arrays.vertices.push_back( cvf::Vec3f( cvf::Vec3d( nodeCoordinates[nodeIdx] ) - m_displayOffset ) );
                        arrays.quadVerticesToNodeIdx.push_back( nodeIdx );
                        arrays.quadVerticesToGlobalElmNodeIdx.push_back( m_part->elementNodeResultIdx( elmIdx, localElmNodeIndicesForFace[n] ) );
                        arrays.quadVerticesToGlobalElmFaceNodeIdx.push_back( elmNodFaceResIdxFaceStart + n );
                        arrays.quadVerticesToGlobalElmIdx.push_back( elmIdx );
                    }

                    arrays.trianglesToElements.push_back( elmIdx );
                    arrays.trianglesToElements.push_back( elmIdx );
                    arrays.trianglesToElementFaces.push_back( lfIdx );
                    arrays.trianglesToElementFaces.push_back( lfIdx );
                }
                else
                {
                    // Handle triangles and 6 node and 8 node faces
                }
            }
        }
    }

    size_t quadVertexCount = 0;
    for ( const QuadArrays& arrays : blockArrays )
    {
        quadVertexCount += arrays.vertices.size();
    }

    std::vector<Vec3f> vertices;
    vertices.reserve( quadVertexCount );
    m_quadVerticesToNodeIdx.reserve( quadVertexCount );
    m_quadVerticesToGlobalElmNodeIdx.reserve( quadVertexCount );
    m_quadVerticesToGlobalElmFaceNodeIdx.reserve( quadVertexCount );
    m_quadVerticesToGlobalElmIdx.reserve( quadVertexCount );
    trianglesToElements.reserve( quadVertexCount / 2 );
    trianglesToElementFaces.reserve( quadVertexCount / 2 );

    auto append = []( auto& target, auto& source )
    {
        target.insert( target.end(), source.begin(), source.end() );

        // Release the per block memory as soon as it has been copied
        std::remove_reference_t<decltype( source )>().swap( source );
    };

    for ( QuadArrays& arrays : blockArrays )
    {
        append( vertices, arrays.vertices );
        append( m_quadVerticesToNodeIdx, arrays.quadVerticesToNodeIdx );
        append( m_quadVerticesToGlobalElmNodeIdx, arrays.quadVerticesToGlobalElmNodeIdx );
        append( m_quadVerticesToGlobalElmFaceNodeIdx, arrays.quadVerticesToGlobalElmFaceNodeIdx );
        append( m_quadVerticesToGlobalElmIdx, arrays.quadVerticesToGlobalElmIdx );
        append( trianglesToElements, arrays.trianglesToElements );
        append( trianglesToElementFaces, arrays.trianglesToElementFaces );
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
cvf::ref<cvf::DrawableGeo> RivFemPartGeometryGenerator::createMeshDrawableFromSingleElement( const RigFemPart* part,
                                                                                             size_t            elmIdx,
                                                                                             const cvf::Vec3d& displayModelOffset,
                                                                                             const std::vector<cvf::Vec3f>& displacements,
                                                                                             double displacementScaleFactor )
{
    cvf::ref<cvf::Vec3fArray> quadVertices;

    {
        std::vector<Vec3f> vertices;

        const std::vector<cvf::Vec3f>& nodeCoordinates  = part->nodes().coordinates;
        bool                           useDisplacements = !displacements.empty() && displacements.size() == nodeCoordinates.size();

        auto nodePosition = [&]( int nodeIdx ) -> cvf::Vec3d
        {
            cvf::Vec3d pos = cvf::Vec3d( nodeCoordinates[nodeIdx] );
            if ( useDisplacements ) pos += cvf::Vec3d( displacements[nodeIdx] ) * displacementScaleFactor;
            return pos;
        };

        RigElementType eType     = part->elementType( elmIdx );
        int            faceCount = RigFemTypes::elementFaceCount( eType );

        const int* elmNodeIndices = part->connectivities( elmIdx );

        for ( int lfIdx = 0; lfIdx < faceCount; ++lfIdx )
        {
            int        faceNodeCount              = 0;
            const int* localElmNodeIndicesForFace = RigFemTypes::localElmNodeIndicesForFace( eType, lfIdx, &faceNodeCount );
            if ( faceNodeCount == 4 )
            {
                vertices.push_back( cvf::Vec3f( nodePosition( elmNodeIndices[localElmNodeIndicesForFace[0]] ) - displayModelOffset ) );
                vertices.push_back( cvf::Vec3f( nodePosition( elmNodeIndices[localElmNodeIndicesForFace[1]] ) - displayModelOffset ) );
                vertices.push_back( cvf::Vec3f( nodePosition( elmNodeIndices[localElmNodeIndicesForFace[2]] ) - displayModelOffset ) );
                vertices.push_back( cvf::Vec3f( nodePosition( elmNodeIndices[localElmNodeIndicesForFace[3]] ) - displayModelOffset ) );
            }
            else
            {
                // Handle triangles and 6 node and 8 node faces
            }
        }

        quadVertices = new cvf::Vec3fArray;
        quadVertices->assign( vertices );
    }

    if ( !quadVertices.notNull() || quadVertices->size() == 0 ) return nullptr;

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray( quadVertices.p() );

    ref<UIntArray> indices = cvf::StructGridGeometryGenerator::lineIndicesFromQuadVertexArray( quadVertices.p() );

    ref<PrimitiveSetIndexedUInt> prim = new PrimitiveSetIndexedUInt( PT_LINES );
    prim->setIndices( indices.p() );

    geo->addPrimitiveSet( prim.p() );
    return geo;
}
