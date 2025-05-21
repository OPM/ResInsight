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

#include "RigTexturedSection.h"
#include "Surface/RigSurfaceResampler.h"

#include "RivAnnotationSourceInfo.h"
#include "RivObjectSourceInfo.h"
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
{
    CVF_ASSERT( section );
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

        cvf::ref<cvf::Part> quadPart = createSingleTexturedQuadPart( displayPoints, part.texture, m_section->isTransparent() );

        cvf::ref<RivSeismicSectionSourceInfo> si = new RivSeismicSectionSourceInfo( m_section, i );
        quadPart->setSourceInfo( si.p() );

        model->addPart( quadPart.p() );
    }
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

        // Create a line segment if we did not find an intersection point or if we are at the end of the polyline
        if ( !foundMatch || ( point == resampledPolyline.back() ) )
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

        std::vector<cvf::Vec3d> completePolyLine;
        cvf::Part*              firstPart = nullptr;

        auto texSection = m_section->texturedSection();
        for ( int i = 0; i < texSection->partsCount(); i++ )
        {
            const auto& texturePart = texSection->part( i );

            // Each part of the seismic section is a rectangle, use two corners of the rectangle to create a polyline
            std::vector<cvf::Vec3d> polyLineForSection = { texturePart.rect[0], texturePart.rect[1] };

            bool closePolyLine         = false;
            auto polyLineDisplayCoords = projectPolyLineOntoSurface( polyLineForSection, surface, displayCoordTransform );

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

            if ( !firstPart ) firstPart = part.p();
            for ( const auto& coords : polyLineDisplayCoords )
            {
                completePolyLine.insert( completePolyLine.end(), coords.begin(), coords.end() );
            }
        }

        if ( firstPart )
        {
            // Add annotation info to be used to display label in Rim3dView::onViewNavigationChanged()
            // Set the source info on one part only, as this data is only used for display of labels
            auto annoObj = new RivAnnotationSourceInfo( surface->fullName().toStdString(), completePolyLine );
            annoObj->setLabelPositionStrategyHint( RivAnnotationTools::LabelPositionStrategy::RIGHT );
            annoObj->setShowColor( true );
            annoObj->setColor( surface->color() );

            firstPart->setSourceInfo( annoObj );
        }
    }
}
