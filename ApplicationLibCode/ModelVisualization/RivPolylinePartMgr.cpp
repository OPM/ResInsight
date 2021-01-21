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

#include "RivPolylinePartMgr.h"

#include "RiaBoundingBoxTools.h"
#include "RiaGuiApplication.h"

#include "Rim3dView.h"
#include "RimEclipseView.h"
#include "RimShowPolylinesInterface.h"

#include "RigMainGrid.h"
#include "RigPolyLinesData.h"

#include "RivPartPriority.h"
#include "RivPolylineGenerator.h"

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
RivPolylinePartMgr::RivPolylinePartMgr( Rim3dView*                 view,
                                        RimShowPolylinesInterface* polylineInterface,
                                        caf::PdmObject*            collection )
    : m_rimView( view )
    , m_polylineInterface( polylineInterface )
    , m_viewCollection( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivPolylinePartMgr::~RivPolylinePartMgr()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivPolylinePartMgr::isPolylinesInBoundingBox( std::vector<std::vector<cvf::Vec3d>> polyline,
                                                   const cvf::BoundingBox&              boundingBox )
{
    auto effectiveBoundingBox = RiaBoundingBoxTools::inflate( boundingBox, 3 );
    for ( const auto& pts : polyline )
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
void RivPolylinePartMgr::buildPolylineParts( const caf::DisplayCoordTransform* displayXf,
                                             const cvf::BoundingBox&           boundingBox )
{
    auto polylineDef = m_polylineInterface->polyLines();
    if ( polylineDef.isNull() || polylineDef->polyLines().size() == 0 )
    {
        clearAllGeometry();
        return;
    }

    auto linesInDomain = getPolylinesPointsInDomain( polylineDef->lockToZPlane(), polylineDef->lockedZValue() );

    if ( !isPolylinesInBoundingBox( linesInDomain, boundingBox ) ) return;

    auto linesInDisplay = transformPolylinesPointsToDisplay( linesInDomain, displayXf );

    clearAllGeometry();

    // Line part
    if ( polylineDef->showLines() )
    {
        cvf::ref<cvf::DrawableGeo> drawableGeo =
            RivPolylineGenerator::createLineAlongPolylineDrawable( linesInDisplay, polylineDef->closePolyline() );
        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName( "RivPolylinePartMgr" );
        part->setDrawable( drawableGeo.p() );

        caf::MeshEffectGenerator effgen( polylineDef->lineColor() );
        effgen.setLineWidth( polylineDef->lineThickness() );
        // if ( isDashedLine ) effgen.setLineStipple( true );
        cvf::ref<cvf::Effect> eff = effgen.generateCachedEffect();

        part->setEffect( eff.p() );
        part->setPriority( RivPartPriority::PartType::MeshLines );

        m_linePart = part;
    }

    // Sphere part
    if ( polylineDef->showSpheres() )
    {
        auto   sphereColor        = polylineDef->sphereColor();
        double sphereRadiusFactor = polylineDef->sphereRadiusFactor();

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

        double cellRadius  = 15.0;
        auto   eclipseView = dynamic_cast<RimEclipseView*>( m_rimView.p() );
        if ( eclipseView )
        {
            double characteristicCellSize = eclipseView->mainGrid()->characteristicIJCellSize();
            cellRadius                    = sphereRadiusFactor * characteristicCellSize;
        }

        cvf::GeometryBuilderTriangles builder;
        cvf::GeometryUtils::createSphere( cellRadius, 15, 15, &builder );
        vectorDrawable->setGlyph( builder.trianglesUShort().p(), builder.vertices().p() );

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName( "RivPolylinePartMgr" );
        part->setDrawable( vectorDrawable.p() );

        part->setEffect( new cvf::Effect() );
        part->setPriority( RivPartPriority::PartType::MeshLines );

        m_spherePart = part;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<cvf::Vec3d>> RivPolylinePartMgr::getPolylinesPointsInDomain( bool snapToPlaneZ, double planeZ )
{
    auto polylines = m_polylineInterface->polyLines()->polyLines();
    if ( !snapToPlaneZ ) return polylines;

    std::vector<std::vector<cvf::Vec3d>> polylinesInDisplay;
    for ( const auto& pts : polylines )
    {
        std::vector<cvf::Vec3d> polyline;
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
    RivPolylinePartMgr::transformPolylinesPointsToDisplay( const std::vector<std::vector<cvf::Vec3d>>& pointsInDomain,
                                                           const caf::DisplayCoordTransform*           displayXf )
{
    std::vector<std::vector<cvf::Vec3d>> pointsInDisplay;
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
void RivPolylinePartMgr::clearAllGeometry()
{
    m_linePart   = nullptr;
    m_spherePart = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivPolylinePartMgr::collectionVisible()
{
    if ( m_viewCollection && m_viewCollection->objectToggleField() )
    {
        caf::PdmField<bool>* field = dynamic_cast<caf::PdmField<bool>*>( m_viewCollection->objectToggleField() );
        return field->value();
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivPolylinePartMgr::appendDynamicGeometryPartsToModel( cvf::ModelBasicList*              model,
                                                            const caf::DisplayCoordTransform* displayXf,
                                                            const cvf::BoundingBox&           boundingBox )
{
    if ( !collectionVisible() ) return;

    // build the lines
    buildPolylineParts( displayXf, boundingBox );

    // add the things we should
    if ( m_linePart.notNull() )
    {
        model->addPart( m_linePart.p() );
    }

    if ( m_spherePart.notNull() )
    {
        model->addPart( m_spherePart.p() );
    }
}
