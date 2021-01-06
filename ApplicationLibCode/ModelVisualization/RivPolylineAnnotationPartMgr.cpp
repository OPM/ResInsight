/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RivPolylineAnnotationPartMgr.h"

#include "RiaBoundingBoxTools.h"
#include "RiaGuiApplication.h"

#include "Rim3dView.h"
#include "RimAnnotationCollection.h"
#include "RimAnnotationInViewCollection.h"
#include "RimAnnotationLineAppearance.h"
#include "RimEclipseView.h"
#include "RimPolylinesAnnotation.h"
#include "RimPolylinesAnnotationInView.h"

#include "RigMainGrid.h"
#include "RigPolyLinesData.h"

#include "RivPartPriority.h"
#include "RivPolylineGenerator.h"
#include "RivPolylinesAnnotationSourceInfo.h"

#include "cafEffectGenerator.h"

#include "cafDisplayCoordTransform.h"
#include "cvfDrawableGeo.h"
#include "cvfDrawableText.h"
#include "cvfDrawableVectors.h"
#include "cvfGeometryBuilderTriangles.h"
#include "cvfGeometryUtils.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfTransform.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivPolylineAnnotationPartMgr::RivPolylineAnnotationPartMgr( Rim3dView* view, RimPolylinesAnnotationInView* annotationInView )
    : m_rimView( view )
    , m_rimAnnotationInView( annotationInView )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivPolylineAnnotationPartMgr::~RivPolylineAnnotationPartMgr()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivPolylineAnnotationPartMgr::buildPolylineAnnotationParts( const caf::DisplayCoordTransform* displayXf )
{
    clearAllGeometry();

    auto rimAnnotation = m_rimAnnotationInView->sourceAnnotation();
    if ( !rimAnnotation->isEmpty() && rimAnnotation->isActive() )
    {
        auto lineColor     = rimAnnotation->appearance()->color();
        auto isDashedLine  = rimAnnotation->appearance()->isDashed();
        auto lineThickness = rimAnnotation->appearance()->thickness();

        auto* collection = annotationCollection();
        if ( !collection ) return;

        auto linesInDomain = getPolylinesPointsInDomain( collection->snapAnnotations(), collection->annotationPlaneZ() );
        auto linesInDisplay = transformPolylinesPointsToDisplay( linesInDomain, displayXf );

        // Line part
        if ( rimAnnotation->showLines() )
        {
            cvf::ref<cvf::DrawableGeo> drawableGeo =
                RivPolylineGenerator::createLineAlongPolylineDrawable( linesInDisplay, rimAnnotation->closePolyline() );
            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName( "RivPolylineAnnotationPartMgr" );
            part->setDrawable( drawableGeo.p() );

            caf::MeshEffectGenerator effgen( lineColor );
            effgen.setLineWidth( lineThickness );
            if ( isDashedLine ) effgen.setLineStipple( true );
            cvf::ref<cvf::Effect> eff = effgen.generateCachedEffect();

            part->setEffect( eff.p() );
            part->setPriority( RivPartPriority::PartType::MeshLines );

            cvf::ref<RivPolylinesAnnotationSourceInfo> sourceInfo = new RivPolylinesAnnotationSourceInfo( rimAnnotation );
            part->setSourceInfo( sourceInfo.p() );

            m_linePart = part;
        }

        // Sphere part
        if ( rimAnnotation->showSpheres() )
        {
            auto   sphereColor        = rimAnnotation->appearance()->sphereColor();
            double sphereRadiusFactor = rimAnnotation->appearance()->sphereRadiusFactor();

            cvf::ref<cvf::Vec3fArray>   vertices = new cvf::Vec3fArray;
            cvf::ref<cvf::Vec3fArray>   vecRes   = new cvf::Vec3fArray;
            cvf::ref<cvf::Color3fArray> colors   = new cvf::Color3fArray;

            size_t pointCount = 0;
            for ( const auto& line : linesInDisplay )
                pointCount += line.size();
            vertices->reserve( pointCount );
            vecRes->reserve( pointCount );
            colors->reserve( pointCount );

            for ( const auto& line : linesInDisplay )
            {
                for ( const auto& v : line )
                {
                    vertices->add( cvf::Vec3f( v ) );
                    vecRes->add( cvf::Vec3f::X_AXIS );
                    colors->add( sphereColor );
                }
            }

            cvf::ref<cvf::DrawableVectors> vectorDrawable;
            if ( RiaGuiApplication::instance()->useShaders() )
            {
                // NOTE: Drawable vectors must be rendered using shaders when the rest of the application is rendered
                // using shaders Drawing vectors using fixed function when rest of the application uses shaders causes
                // visual artifacts
                vectorDrawable = new cvf::DrawableVectors( "u_transformationMatrix", "u_color" );
            }
            else
            {
                vectorDrawable = new cvf::DrawableVectors();
            }

            vectorDrawable->setVectors( vertices.p(), vecRes.p() );
            vectorDrawable->setColors( colors.p() );

            cvf::GeometryBuilderTriangles builder;
            double                        cellRadius  = 15.0;
            auto                          eclipseView = dynamic_cast<RimEclipseView*>( m_rimView.p() );
            if ( eclipseView )
            {
                double characteristicCellSize = eclipseView->mainGrid()->characteristicIJCellSize();
                cellRadius                    = sphereRadiusFactor * characteristicCellSize;
            }

            cvf::GeometryUtils::createSphere( cellRadius, 15, 15, &builder );
            vectorDrawable->setGlyph( builder.trianglesUShort().p(), builder.vertices().p() );

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName( "RivPolylineAnnotationPartMgr" );
            part->setDrawable( vectorDrawable.p() );

            part->setEffect( new cvf::Effect() );
            part->setPriority( RivPartPriority::PartType::MeshLines );

            cvf::ref<RivPolylinesAnnotationSourceInfo> sourceInfo = new RivPolylinesAnnotationSourceInfo( rimAnnotation );
            part->setSourceInfo( sourceInfo.p() );

            m_spherePart = part;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<RivPolylineAnnotationPartMgr::Vec3d>>
    RivPolylineAnnotationPartMgr::getPolylinesPointsInDomain( bool snapToPlaneZ, double planeZ )
{
    auto polylines = m_rimAnnotationInView->sourceAnnotation()->polyLinesData()->polyLines();
    if ( !snapToPlaneZ ) return polylines;

    std::vector<std::vector<Vec3d>> polylinesInDisplay;
    for ( const auto& pts : polylines )
    {
        std::vector<Vec3d> polyline;
        for ( const auto& pt : pts )
        {
            auto ptInDisp = pt;
            ptInDisp.z()  = planeZ;
            polyline.push_back( ptInDisp );
        }
        polylinesInDisplay.push_back( polyline );
    }
    return polylinesInDisplay;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<cvf::Vec3d>>
    RivPolylineAnnotationPartMgr::transformPolylinesPointsToDisplay( const std::vector<std::vector<Vec3d>>& pointsInDomain,
                                                                     const caf::DisplayCoordTransform*      displayXf )
{
    std::vector<std::vector<Vec3d>> pointsInDisplay;
    for ( const auto& pts : pointsInDomain )
    {
        std::vector<cvf::Vec3d> displayCoords = displayXf->transformToDisplayCoords( pts );

        pointsInDisplay.push_back( displayCoords );
    }
    return pointsInDisplay;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivPolylineAnnotationPartMgr::isPolylinesInBoundingBox( const cvf::BoundingBox& boundingBox )
{
    auto coll = annotationCollection();
    if ( !coll ) return false;

    auto effectiveBoundingBox = RiaBoundingBoxTools::inflate( boundingBox, 3 );
    for ( const auto& pts : getPolylinesPointsInDomain( coll->snapAnnotations(), coll->annotationPlaneZ() ) )
    {
        for ( const auto& pt : pts )
        {
            if ( effectiveBoundingBox.contains( pt ) ) return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivPolylineAnnotationPartMgr::clearAllGeometry()
{
    m_linePart   = nullptr;
    m_spherePart = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnnotationInViewCollection* RivPolylineAnnotationPartMgr::annotationCollection() const
{
    std::vector<RimAnnotationInViewCollection*> colls;
    m_rimView->descendantsIncludingThisOfType( colls );
    return !colls.empty() ? colls.front() : nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivPolylineAnnotationPartMgr::appendDynamicGeometryPartsToModel( cvf::ModelBasicList*              model,
                                                                      const caf::DisplayCoordTransform* displayXf,
                                                                      const cvf::BoundingBox&           boundingBox )
{
    auto rimAnnotation = m_rimAnnotationInView->sourceAnnotation();
    if ( !rimAnnotation ) return;
    if ( rimAnnotation->isEmpty() ) return;
    if ( !m_rimAnnotationInView->isVisible() ) return;

    // Check bounding box
    if ( !isPolylinesInBoundingBox( boundingBox ) ) return;

    buildPolylineAnnotationParts( displayXf );

    if ( m_linePart.notNull() )
    {
        model->addPart( m_linePart.p() );
    }

    if ( m_spherePart.notNull() )
    {
        model->addPart( m_spherePart.p() );
    }
}
