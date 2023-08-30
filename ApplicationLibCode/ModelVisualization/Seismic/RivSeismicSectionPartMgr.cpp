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

#include "Rim3dView.h"
#include "RimRegularLegendConfig.h"
#include "RimSeismicAlphaMapper.h"
#include "RimSeismicSection.h"
#include "RimSeismicSectionCollection.h"
#include "RimSurface.h"
#include "RimSurfaceCollection.h"
#include "RimTools.h"

#include "RigSurfaceResampler.h"
#include "RigTexturedSection.h"

#include "RivPartPriority.h"
#include "RivPolylineGenerator.h"
#include "RivPolylinePartMgr.h"
#include "RivSeismicSectionSourceInfo.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"
#include "cafPdmObject.h"

#include "cvfLibCore.h"
#include "cvfLibGeometry.h"
#include "cvfLibRender.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfScalarMapper.h"

#include <zgyaccess/seismicslice.h>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivSeismicSectionPartMgr::RivSeismicSectionPartMgr( RimSeismicSection* section )
    : m_section( section )
    , m_canUseShaders( true )
{
    CVF_ASSERT( section );

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
    if ( !m_canUseShaders ) return;

    auto texSection = m_section->texturedSection();

    for ( int i = 0; i < texSection->partsCount(); i++ )
    {
        auto& part = texSection->part( i );

        cvf::Vec3dArray displayPoints;
        displayPoints.reserve( part.rect.size() );

        for ( auto& vOrg : part.rect )
        {
            displayPoints.add( displayCoordTransform->transformToDisplayCoord( vOrg ) );
        }

        if ( part.texture.isNull() )
        {
            if ( ( part.sliceData == nullptr ) || part.sliceData.get()->isEmpty() ) continue;

            part.texture = createImageFromData( part.sliceData.get() );
        }

        cvf::ref<cvf::Part> quadPart = createSingleTexturedQuadPart( displayPoints, part.texture );

        cvf::ref<RivSeismicSectionSourceInfo> si = new RivSeismicSectionSourceInfo( m_section, i );
        quadPart->setSourceInfo( si.p() );

        model->addPart( quadPart.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivSeismicSectionPartMgr::createSingleTexturedQuadPart( const cvf::Vec3dArray&      cornerPoints,
                                                                            cvf::ref<cvf::TextureImage> image )
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

    if ( m_section->isTransparent() )
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
cvf::ref<cvf::DrawableGeo> RivSeismicSectionPartMgr::createXYPlaneQuadGeoWithTexCoords( const cvf::Vec3dArray& cornerPoints )
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::TextureImage* RivSeismicSectionPartMgr::createImageFromData( ZGYAccess::SeismicSliceData* data )
{
    const int width = data->width();
    const int depth = data->depth();

    cvf::TextureImage* textureImage = new cvf::TextureImage();
    textureImage->allocate( width, depth );

    auto   legend = m_section->legendConfig();
    float* pData  = data->values();

    if ( ( legend == nullptr ) || ( pData == nullptr ) )
    {
        textureImage->fill( cvf::Color4ub( 0, 0, 0, 0 ) );
        return textureImage;
    }

    const bool isTransparent = m_section->isTransparent();

    auto alphaMapper = m_section->alphaValueMapper();
    auto colorMapper = legend->scalarMapper();

    for ( int i = 0; i < width; i++ )
    {
        for ( int j = depth - 1; j >= 0; j-- )
        {
            auto rgb = colorMapper->mapToColor( *pData );

            cvf::ubyte uAlpha = 255;
            if ( isTransparent ) uAlpha = alphaMapper->alphaValue( *pData );

            textureImage->setPixel( i, j, cvf::Color4ub( rgb, uAlpha ) );

            pData++;
        }
    }

    return textureImage;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<cvf::Vec3d>>
    RivSeismicSectionPartMgr::projectPolyLineOntoSurface( std::vector<cvf::Vec3d>           polyLine,
                                                          RimSurface*                       surface,
                                                          const caf::DisplayCoordTransform* displayCoordTransform )
{
    std::vector<std::vector<cvf::Vec3d>> displayPolygonCurves;
    const double                         resamplingDistance = 5.0;
    std::vector<cvf::Vec3d>              resampledPolyline  = RigSurfaceResampler::computeResampledPolyline( polyLine, resamplingDistance );

    std::vector<cvf::Vec3d> domainCurvePoints;

    for ( const auto& point : resampledPolyline )
    {
        cvf::Vec3d pointAbove = cvf::Vec3d( point.x(), point.y(), 10000.0 );
        cvf::Vec3d pointBelow = cvf::Vec3d( point.x(), point.y(), -10000.0 );

        cvf::Vec3d intersectionPoint;
        bool foundMatch = RigSurfaceResampler::computeIntersectionWithLine( surface->surfaceData(), pointAbove, pointBelow, intersectionPoint );
        if ( foundMatch )
        {
            domainCurvePoints.emplace_back( intersectionPoint );
        }
        else
        {
            if ( domainCurvePoints.size() > 1 )
            {
                // Add intersection curve in display coordinates
                auto displayCoords = displayCoordTransform->transformToDisplayCoords( domainCurvePoints );
                displayPolygonCurves.push_back( displayCoords );
            }

            // Start a new line
            domainCurvePoints.clear();
        }
    }

    return displayPolygonCurves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivSeismicSectionPartMgr::appendSurfaceIntersectionLines( cvf::ModelBasicList*              model,
                                                               const caf::DisplayCoordTransform* displayCoordTransform,
                                                               double                            lineThickness,
                                                               const std::vector<RimSurface*>&   surfaces )
{
    for ( auto surface : surfaces )
    {
        if ( !surface ) continue;

        surface->loadDataIfRequired();
        if ( !surface->surfaceData() ) continue;

        auto texSection = m_section->texturedSection();
        for ( int i = 0; i < texSection->partsCount(); i++ )
        {
            const auto& texturePart = texSection->part( i );

            std::vector<cvf::Vec3d> polyLine;

            // Each part of the seismic section is a rectangle, use two corners of the rectangle to create a polyline
            polyLine.push_back( texturePart.rect[0] );
            polyLine.push_back( texturePart.rect[1] );

            bool closePolyLine         = false;
            auto polyLineDisplayCoords = projectPolyLineOntoSurface( polyLine, surface, displayCoordTransform );

            cvf::ref<cvf::DrawableGeo> drawableGeo =
                RivPolylineGenerator::createLineAlongPolylineDrawable( polyLineDisplayCoords, closePolyLine );
            if ( drawableGeo.isNull() ) continue;

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName( "RivSeismicSectionPartMgr::SurfaceIntersectionLine" );
            part->setDrawable( drawableGeo.p() );

            caf::MeshEffectGenerator effgen( surface->color() );
            effgen.setLineWidth( lineThickness );
            cvf::ref<cvf::Effect> eff = effgen.generateCachedEffect();

            cvf::ref<cvf::RenderStatePolygonOffset> polyOffset = new cvf::RenderStatePolygonOffset;
            polyOffset->enableFillMode( true );
            polyOffset->setFactor( -5 );
            const double maxOffsetFactor = -1000;
            polyOffset->setUnits( maxOffsetFactor );

            eff->setRenderState( polyOffset.p() );

            part->setEffect( eff.p() );
            part->setPriority( RivPartPriority::PartType::MeshLines );

            model->addPart( part.p() );
        }
    }
}
