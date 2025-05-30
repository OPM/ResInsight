/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RivMeasurementPartMgr.h"

#include "RiaBoundingBoxTools.h"
#include "RiaColorTools.h"
#include "RiaFontCache.h"
#include "RiaGuiApplication.h"
#include "RiaPreferences.h"

#include "Rim3dView.h"
#include "RimMeasurement.h"
#include "RimProject.h"

#include "RiuGuiTheme.h"

#include "RivPartPriority.h"
#include "RivPolylineGenerator.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"
#include "cafFixedAtlasFont.h"

#include "cvfBoundingBox.h"
#include "cvfCamera.h"
#include "cvfDrawableGeo.h"
#include "cvfDrawableText.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfRenderStateDepth.h"
#include "cvfRenderStatePoint.h"
#include "cvfqtUtils.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivMeasurementPartMgr::RivMeasurementPartMgr( Rim3dView* view )
    : m_rimView( view )
    , m_measurement( nullptr )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivMeasurementPartMgr::~RivMeasurementPartMgr()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivMeasurementPartMgr::appendGeometryPartsToModel( const cvf::Camera*                camera,
                                                        cvf::ModelBasicList*              model,
                                                        const caf::DisplayCoordTransform* displayCoordTransform,
                                                        const cvf::BoundingBox&           boundingBox )
{
    if ( m_measurement.isNull() )
    {
        m_measurement = RimProject::current()->measurement();
    }

    if ( m_measurement.isNull() ) return;
    if ( m_measurement->pointsInDomainCoords().empty() ) return;

    // Check bounding box
    if ( !isPolylinesInBoundingBox( boundingBox ) ) return;

    buildPolyLineParts( camera, displayCoordTransform );

    if ( m_linePart.notNull() )
    {
        model->addPart( m_linePart.p() );
    }

    if ( m_pointPart.notNull() )
    {
        model->addPart( m_pointPart.p() );
    }

    if ( m_labelPart.notNull() )
    {
        model->addPart( m_labelPart.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivMeasurementPartMgr::clearGeometryCache()
{
    m_linePart  = nullptr;
    m_pointPart = nullptr;
    m_labelPart = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivMeasurementPartMgr::buildPolyLineParts( const cvf::Camera* camera, const caf::DisplayCoordTransform* displayCoordTransform )
{
    auto pointsInDisplay = displayCoordTransform->transformToDisplayCoords( m_measurement->pointsInDomainCoords() );

    // Measurement lines
    {
        cvf::ref<cvf::DrawableGeo> polylineGeo = RivPolylineGenerator::createLineAlongPolylineDrawable( pointsInDisplay );
        if ( polylineGeo.notNull() )
        {
            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName( "Cross Section Polyline" );
            part->setDrawable( polylineGeo.p() );

            part->updateBoundingBox();
            part->setPriority( RivPartPriority::PartType::Highlight );

            cvf::ref<cvf::Effect>    eff;
            caf::MeshEffectGenerator lineEffGen( cvf::Color3::MAGENTA );
            eff = lineEffGen.generateUnCachedEffect();

            cvf::ref<cvf::RenderStateDepth> depth = new cvf::RenderStateDepth;
            depth->enableDepthTest( false );
            eff->setRenderState( depth.p() );

            part->setEffect( eff.p() );

            m_linePart = part;
        }
    }

    // Measurement points
    cvf::ref<cvf::DrawableGeo> polylinePointsGeo = RivPolylineGenerator::createPointsFromPolylineDrawable( pointsInDisplay );
    if ( polylinePointsGeo.notNull() )
    {
        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName( "Cross Section Polyline" );
        part->setDrawable( polylinePointsGeo.p() );

        part->updateBoundingBox();
        part->setPriority( RivPartPriority::PartType::Highlight );

        cvf::ref<cvf::Effect>    eff;
        caf::MeshEffectGenerator lineEffGen( cvf::Color3::MAGENTA );
        eff = lineEffGen.generateUnCachedEffect();

        cvf::ref<cvf::RenderStateDepth> depth = new cvf::RenderStateDepth;
        depth->enableDepthTest( false );
        eff->setRenderState( depth.p() );

        cvf::ref<cvf::RenderStatePoint> pointRendState = new cvf::RenderStatePoint( cvf::RenderStatePoint::FIXED_SIZE );
        pointRendState->setSize( 5.0f );
        eff->setRenderState( pointRendState.p() );

        part->setEffect( eff.p() );

        m_pointPart = part;
    }

    // Text label
    if ( pointsInDisplay.size() > 1 )
    {
        bool       negativeXDir = false;
        cvf::Vec3d lastV        = pointsInDisplay[pointsInDisplay.size() - 1] - pointsInDisplay[pointsInDisplay.size() - 2];
        if ( lastV.x() < 0.0 )
        {
            negativeXDir = true;
        }
        QString text          = m_measurement->label();
        auto    labelPosition = pointsInDisplay.back();

        auto drawableText = RivPolylineGenerator::createOrientedLabel( negativeXDir, camera, labelPosition, text );

        cvf::ref<cvf::Part> part = new cvf::Part;

        part->setName( "RivMeasurementPartMgr: " );
        part->setDrawable( drawableText.p() );

        cvf::ref<cvf::Effect> eff = new cvf::Effect();
        part->setEffect( eff.p() );
        part->setPriority( RivPartPriority::PartType::Text );

        m_labelPart = part;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivMeasurementPartMgr::isPolylinesInBoundingBox( const cvf::BoundingBox& boundingBox )
{
    auto effectiveBoundingBox = RiaBoundingBoxTools::inflate( boundingBox, 3 );
    for ( const auto& pt : m_measurement->pointsInDomainCoords() )
    {
        if ( effectiveBoundingBox.contains( pt ) ) return true;
    }
    return false;
}
