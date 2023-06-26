/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RivTexturePartMgr.h"

#include "RiaGuiApplication.h"

#include "RivPartPriority.h"

#include "RigTexturedSection.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"
#include "cafPdmObject.h"

#include "cvfLibCore.h"
#include "cvfLibGeometry.h"
#include "cvfLibRender.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfScalarMapper.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivTexturePartMgr::RivTexturePartMgr()
    : m_canUseShaders( true )
{
    m_canUseShaders = RiaGuiApplication::instance()->useShaders();

    cvf::ShaderProgramGenerator gen( "Texturing", cvf::ShaderSourceProvider::instance() );
    gen.addVertexCode( cvf::ShaderSourceRepository::vs_Standard );
    gen.addFragmentCode( cvf::ShaderSourceRepository::src_Texture );
    gen.addFragmentCode( cvf::ShaderSourceRepository::fs_Unlit );
    m_textureShaderProg = gen.generate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivTexturePartMgr::~RivTexturePartMgr()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part>
    RivTexturePartMgr::createSingleTexturedQuadPart( const cvf::Vec3dArray& cornerPoints, cvf::ref<cvf::TextureImage> image, bool transparent )
{
    cvf::ref<cvf::Part> part = new cvf::Part;

    cvf::ref<cvf::DrawableGeo> geo = createXYPlaneQuadGeoWithTexCoords( cornerPoints );

    cvf::ref<cvf::Texture> texture = new cvf::Texture( image.p() );
    cvf::ref<cvf::Sampler> sampler = new cvf::Sampler;
    sampler->setMinFilter( cvf::Sampler::LINEAR );
    sampler->setMagFilter( cvf::Sampler::NEAREST );
    sampler->setWrapModeS( cvf::Sampler::CLAMP_TO_EDGE );
    sampler->setWrapModeT( cvf::Sampler::CLAMP_TO_EDGE );

    cvf::ref<cvf::RenderStateTextureBindings> textureBindings = new cvf::RenderStateTextureBindings;
    textureBindings->addBinding( texture.p(), sampler.p(), "u_texture2D" );

    cvf::ref<cvf::Effect> eff = new cvf::Effect;
    eff->setRenderState( textureBindings.p() );
    eff->setShaderProgram( m_textureShaderProg.p() );

    if ( transparent )
    {
        part->setPriority( RivPartPriority::PartType::TransparentSeismic );
        cvf::ref<cvf::RenderStateBlending> blending = new cvf::RenderStateBlending;
        blending->configureTransparencyBlending();
        eff->setRenderState( blending.p() );
    }

    part->setDrawable( geo.p() );
    part->setEffect( eff.p() );

    return part;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivTexturePartMgr::createXYPlaneQuadGeoWithTexCoords( const cvf::Vec3dArray& cornerPoints )
{
    cvf::ref<cvf::Vec3fArray> vertices = new cvf::Vec3fArray;
    vertices->reserve( 4 );

    for ( const auto& v : cornerPoints )
    {
        vertices->add( cvf::Vec3f( v ) );
    }

    cvf::ref<cvf::Vec2fArray> texCoords = new cvf::Vec2fArray;
    texCoords->reserve( 4 );
    texCoords->add( cvf::Vec2f( 0, 0 ) );
    texCoords->add( cvf::Vec2f( 1, 0 ) );
    texCoords->add( cvf::Vec2f( 1, 1 ) );
    texCoords->add( cvf::Vec2f( 0, 1 ) );

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->setVertexArray( vertices.p() );
    geo->setTextureCoordArray( texCoords.p() );

    cvf::ref<cvf::UIntArray> indices = new cvf::UIntArray;
    indices->reserve( 6 );

    for ( uint i : { 0, 1, 2, 0, 2, 3 } )
    {
        indices->add( i );
    }

    cvf::ref<cvf::PrimitiveSetIndexedUInt> primSet = new cvf::PrimitiveSetIndexedUInt( cvf::PT_TRIANGLES );
    primSet->setIndices( indices.p() );
    geo->addPrimitiveSet( primSet.p() );

    geo->computeNormals();

    return geo;
}
