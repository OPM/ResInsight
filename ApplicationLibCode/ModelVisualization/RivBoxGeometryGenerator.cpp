/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RivBoxGeometryGenerator.h"

#include "cafEffectGenerator.h"

#include "cvfBase.h"
#include "cvfDrawableGeo.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetDirect.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfStructGridGeometryGenerator.h"
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivBoxGeometryGenerator::createBoxFromVertices( const std::vector<cvf::Vec3f>& vertices,
                                                                    const cvf::Color3f             color )
{
    std::vector<cvf::Vec3f> boxVertices;

    for ( int enumInt = cvf::StructGridInterface::POS_I; enumInt < cvf::StructGridInterface::NO_FACE; enumInt++ )
    {
        cvf::StructGridInterface::FaceType face = static_cast<cvf::StructGridInterface::FaceType>( enumInt );

        cvf::ubyte faceConn[4];
        cvf::StructGridInterface::cellFaceVertexIndices( face, faceConn );

        for ( int n = 0; n < 4; n++ )
        {
            boxVertices.push_back( cvf::Vec3f( vertices[faceConn[n]] ) );
        }
    }

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;

    cvf::ref<cvf::Vec3fArray> cvfVertices = new cvf::Vec3fArray;
    cvfVertices->assign( boxVertices );

    if ( !( cvfVertices.notNull() && cvfVertices->size() != 0 ) ) return nullptr;

    geo->setVertexArray( cvfVertices.p() );

    cvf::ref<cvf::UIntArray> indices = lineIndicesFromQuadVertexArray( cvfVertices.p() );

    cvf::ref<cvf::PrimitiveSetIndexedUInt> prim = new cvf::PrimitiveSetIndexedUInt( cvf::PT_LINES );
    prim->setIndices( indices.p() );

    geo->addPrimitiveSet( prim.p() );

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setName( cvf::String( "RivBoxGeometryGenerator::createBoxFromVertices" ) );

    part->setDrawable( geo.p() );

    cvf::ref<cvf::Effect>    eff;
    caf::MeshEffectGenerator effGen( color );
    eff = effGen.generateUnCachedEffect();
    part->setEffect( eff.p() );

    return part;
}

cvf::ref<cvf::UIntArray> RivBoxGeometryGenerator::lineIndicesFromQuadVertexArray( const cvf::Vec3fArray* vertexArray )
{
    // TODO - see issue #7890

    CVF_ASSERT( vertexArray );

    size_t numVertices = vertexArray->size();
    int    numQuads    = static_cast<int>( numVertices / 4 );
    CVF_ASSERT( numVertices % 4 == 0 );

    cvf::ref<cvf::UIntArray> indices = new cvf::UIntArray;
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
