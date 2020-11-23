/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 - Equinor ASA
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

#include "RivPolylineSelectionPartMgr.h"

#include "RiaBoundingBoxTools.h"
#include "RiaGuiApplication.h"

#include "Rim3dView.h"
#include "RimCellFilterCollection.h"
#include "RimEclipseView.h"

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
RivPolylineSelectionPartMgr::RivPolylineSelectionPartMgr( Rim3dView* view )
    : m_rimView( view )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivPolylineSelectionPartMgr::~RivPolylineSelectionPartMgr()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivPolylineSelectionPartMgr::buildPolylineParts( const caf::DisplayCoordTransform* displayXf )
{
    clearAllGeometry();

    auto selectionpolygon = m_cellfilterCollection->selectedPolygon();
    if ( selectionpolygon && selectionpolygon->polyLines().size() > 0 )
    {
        cvf::Color3f lineColor( 255, 255, 255 );
        bool         isDashedLine  = false;
        auto         lineThickness = 2.0;

        auto linesInDomain  = selectionpolygon->polyLines()[0];
        auto linesInDisplay = displayXf->transformToDisplayCoords( linesInDomain );

        // Line part
        {
            cvf::ref<cvf::DrawableGeo> drawableGeo =
                RivPolylineGenerator::createLineAlongPolylineDrawable( linesInDisplay, true );
            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName( "RivPolylineSelectionPartMgr" );
            part->setDrawable( drawableGeo.p() );

            caf::MeshEffectGenerator effgen( lineColor );
            effgen.setLineWidth( lineThickness );
            if ( isDashedLine ) effgen.setLineStipple( true );
            cvf::ref<cvf::Effect> eff = effgen.generateCachedEffect();

            part->setEffect( eff.p() );
            part->setPriority( RivPartPriority::PartType::MeshLines );

            // part->setSourceInfo( sourceInfo.p() );

            m_linePart = part;
        }
        // Sphere part
        {
            auto   sphereColor        = lineColor;
            double sphereRadiusFactor = 1.0;

            cvf::ref<cvf::Vec3fArray>   vertices = new cvf::Vec3fArray;
            cvf::ref<cvf::Vec3fArray>   vecRes   = new cvf::Vec3fArray;
            cvf::ref<cvf::Color3fArray> colors   = new cvf::Color3fArray;

            size_t pointCount = linesInDisplay.size();
            vertices->reserve( pointCount );
            vecRes->reserve( pointCount );
            colors->reserve( pointCount );

            for ( const auto& v : linesInDisplay )
            {
                vertices->add( cvf::Vec3f( v ) );
                vecRes->add( cvf::Vec3f::X_AXIS );
                colors->add( sphereColor );
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
            part->setName( "RivPolylineSelectionPartMgr" );
            part->setDrawable( vectorDrawable.p() );

            part->setEffect( new cvf::Effect() );
            part->setPriority( RivPartPriority::PartType::MeshLines );

            // part->setSourceInfo( sourceInfo.p() );

            m_spherePart = part;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivPolylineSelectionPartMgr::isPolylinesInBoundingBox( const cvf::BoundingBox& boundingBox )
{
    auto coll = cellFilterCollection();
    if ( !coll ) return false;
    auto polygon = coll->selectedPolygon();
    if ( !polygon ) return false;
    if ( polygon->polyLines().size() == 0 ) return false;

    auto effectiveBoundingBox = RiaBoundingBoxTools::inflate( boundingBox, 3 );
    for ( const auto& pt : polygon->polyLines()[0] )
    {
        if ( effectiveBoundingBox.contains( pt ) ) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivPolylineSelectionPartMgr::clearAllGeometry()
{
    m_linePart   = nullptr;
    m_spherePart = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellFilterCollection* RivPolylineSelectionPartMgr::cellFilterCollection() const
{
    std::vector<RimCellFilterCollection*> colls;
    m_rimView->descendantsIncludingThisOfType( colls );
    return !colls.empty() ? colls.front() : nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivPolylineSelectionPartMgr::appendDynamicGeometryPartsToModel( cvf::ModelBasicList*              model,
                                                                     const caf::DisplayCoordTransform* displayXf,
                                                                     const cvf::BoundingBox&           boundingBox )
{
    m_cellfilterCollection = cellFilterCollection();

    if ( m_cellfilterCollection.isNull() ) return;
    if ( m_cellfilterCollection->isEmpty() ) return;
    if ( !m_cellfilterCollection->isActive() ) return;

    // Check bounding box
    if ( !isPolylinesInBoundingBox( boundingBox ) ) return;

    buildPolylineParts( displayXf );

    if ( m_linePart.notNull() )
    {
        model->addPart( m_linePart.p() );
    }

    if ( m_spherePart.notNull() )
    {
        model->addPart( m_spherePart.p() );
    }
}
