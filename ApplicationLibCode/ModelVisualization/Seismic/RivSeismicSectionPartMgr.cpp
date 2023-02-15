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

#include "RivSeismicSectionPartMgr.h"

#include "RiaGuiApplication.h"

#include "RivPolylinePartMgr.h"

#include "Rim3dView.h"
#include "RimGridView.h"
#include "RimSeismicSection.h"
#include "RimSeismicSectionCollection.h"

#include "RigTexturedSection.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"
#include "cafPdmObject.h"

#include "cvfLibCore.h"
#include "cvfLibGeometry.h"
#include "cvfLibRender.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivSeismicSectionPartMgr::RivSeismicSectionPartMgr( RimSeismicSection* section )
    : m_section( section )
    , m_canUseShaders( true )
{
    CVF_ASSERT( section );

    m_canUseShaders = RiaGuiApplication::instance()->useShaders();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivSeismicSectionPartMgr::appendPolylinePartsToModel( Rim3dView*                        view,
                                                           cvf::ModelBasicList*              model,
                                                           const caf::DisplayCoordTransform* transform,
                                                           const cvf::BoundingBox&           boundingBox )
{
    if ( m_polylinePartMgr.isNull() ) m_polylinePartMgr = new RivPolylinePartMgr( view, m_section.p(), m_section.p() );

    m_polylinePartMgr->appendDynamicGeometryPartsToModel( model, transform, boundingBox );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivSeismicSectionPartMgr::appendGeometryPartsToModel( cvf::ModelBasicList*              model,
                                                           const caf::DisplayCoordTransform* displayCoordTransform,
                                                           const cvf::BoundingBox&           boundingBox )
{
    auto texSection = m_section->texturedSection();

    if ( m_canUseShaders )
    {
        auto& rects = texSection->rects();

        for ( int i = 0; i < (int)rects.size(); i++ )
        {
            cvf::Vec3dArray transformedPoints;
            transformedPoints.reserve( rects[i].size() );

            for ( auto& vOrg : rects[i] )
            {
                transformedPoints.add( displayCoordTransform->transformToDisplayCoord( vOrg ) );
            }

            cvf::ref<cvf::Part> quadPart =
                createSingleTexturedQuadPart( transformedPoints, texSection->width( i ), texSection->height() );
            model->addPart( quadPart.p() );
        }
    }

    model->updateBoundingBoxesRecursive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part>
    RivSeismicSectionPartMgr::createSingleTexturedQuadPart( const cvf::Vec3dArray& cornerPoints, int width, int height )
{
    cvf::ref<cvf::DrawableGeo> geo = createXYPlaneQuadGeoWithTexCoords( cornerPoints );

    cvf::ref<cvf::Effect> eff = new cvf::Effect;

    {
        cvf::ref<cvf::TextureImage> textureImage = new cvf::TextureImage();

        textureImage->allocate( width, height );
        textureImage->fill( cvf::Color4ub( 255, 255, 255, 128 ) );

        cvf::ref<cvf::Texture> texture = new cvf::Texture( textureImage.p() );
        cvf::ref<cvf::Sampler> sampler = new cvf::Sampler;
        sampler->setMinFilter( cvf::Sampler::LINEAR );
        sampler->setMagFilter( cvf::Sampler::NEAREST );
        sampler->setWrapModeS( cvf::Sampler::CLAMP_TO_EDGE );
        sampler->setWrapModeT( cvf::Sampler::CLAMP_TO_EDGE );

        cvf::ref<cvf::RenderStateTextureBindings> textureBindings = new cvf::RenderStateTextureBindings;
        textureBindings->addBinding( texture.p(), sampler.p(), "u_texture2D" );

        eff->setRenderState( textureBindings.p() );
    }

    bool useShaderProgram = true;
    if ( useShaderProgram )
    {
        cvf::ShaderProgramGenerator gen( "Texturing", cvf::ShaderSourceProvider::instance() );
        gen.addVertexCode( cvf::ShaderSourceRepository::vs_Standard );
        gen.addFragmentCode( cvf::ShaderSourceRepository::src_Texture );
        gen.addFragmentCode( cvf::ShaderSourceRepository::light_SimpleHeadlight );
        gen.addFragmentCode( cvf::ShaderSourceRepository::fs_Standard );
        cvf::ref<cvf::ShaderProgram> prog = gen.generate();

        eff->setShaderProgram( prog.p() );
    }

    eff->setRenderState( new cvf::RenderStateMaterial_FF( cvf::Color3f( 1, 1, 1 ) ) );

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setDrawable( geo.p() );
    part->setEffect( eff.p() );

    return part;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivSeismicSectionPartMgr::createXYPlaneQuadGeoWithTexCoords( const cvf::Vec3dArray& cornerPoints )
{
    cvf::ref<cvf::Vec3fArray> vertices  = new cvf::Vec3fArray;
    cvf::ref<cvf::Vec2fArray> texCoords = new cvf::Vec2fArray;
    vertices->reserve( 4 );
    texCoords->reserve( 4 );

    for ( auto& v : cornerPoints )
    {
        vertices->add( cvf::Vec3f( v ) );
    }

    texCoords->add( cvf::Vec2f( 0, 0 ) );
    texCoords->add( cvf::Vec2f( 1, 0 ) );
    texCoords->add( cvf::Vec2f( 1, 1 ) );
    texCoords->add( cvf::Vec2f( 0, 1 ) );

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->setVertexArray( vertices.p() );
    geo->setTextureCoordArray( texCoords.p() );

    cvf::ref<cvf::UIntArray> indices = new cvf::UIntArray;
    indices->reserve( 6 );
    indices->add( 0 );
    indices->add( 1 );
    indices->add( 2 );
    indices->add( 0 );
    indices->add( 2 );
    indices->add( 3 );
    cvf::ref<cvf::PrimitiveSetIndexedUInt> primSet = new cvf::PrimitiveSetIndexedUInt( cvf::PT_TRIANGLES );
    primSet->setIndices( indices.p() );
    geo->addPrimitiveSet( primSet.p() );

    geo->computeNormals();

    return geo;
}
