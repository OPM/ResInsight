/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimContourMapTopsCollection.h"

#include "RimContourMapTop.h"

#include "ContourMap/RigContourMapProjection.h"
#include "ContourMap/RigContourMapTopFinder.h"
#include "ContourMap/RimContourMapProjection.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonCollection.h"
#include "Polygons/RimPolygonInViewCollection.h"

#include "RimGridView.h"
#include "RimProject.h"
#include "RimTools.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmUiOrdering.h"

#include <algorithm>

CAF_PDM_SOURCE_INIT( RimContourMapTopsCollection, "RimContourMapTopsCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimContourMapTopsCollection::RimContourMapTopsCollection()
{
    CAF_PDM_InitObject( "Detected Tops", ":/WellTargetPoint16x16.png" );

    CAF_PDM_InitField( &m_topCount, "TopCount", 10, "Number of Tops" );
    CAF_PDM_InitField( &m_minDistance, "MinDistance", 0.0, "Minimum Distance" );

    CAF_PDM_InitFieldNoDefault( &m_tops, "Tops", "Tops" );

    nameField()->uiCapability()->setUiReadOnly( true );
    setName( "Detected Tops" );
}

//--------------------------------------------------------------------------------------------------
/// Creates both the RimContourMapTop metadata object and its single-point marker polygon (used to
/// visualize the top's position as a sphere, reusing the existing polygon rendering/mirroring machinery).
//--------------------------------------------------------------------------------------------------
RimContourMapTop*
    RimContourMapTopsCollection::addTop( int rank, double value, double prominence, const cvf::Vec3d& domainPosition, double sphereRadiusFactor )
{
    auto polygonCollection = RimTools::polygonCollection();
    if ( !polygonCollection ) return nullptr;

    auto* top = new RimContourMapTop;
    top->setValues( rank, value, prominence, domainPosition );

    auto* markerPolygon = polygonCollection->appendUserDefinedPolygon();
    markerPolygon->setPointsInDomainCoords( { domainPosition } );
    markerPolygon->setName( top->name() );
    markerPolygon->setReadOnly( true );
    markerPolygon->setShowLines( false );
    markerPolygon->setShowSpheres( true );
    markerPolygon->setSphereRadiusFactor( sphereRadiusFactor );
    markerPolygon->setSphereColor( cvf::Color3f::DEEP_PINK );
    markerPolygon->coordinatesChanged.send();

    top->setMarkerPolygon( markerPolygon );

    m_tops.push_back( top );

    return top;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapTopsCollection::clearTops()
{
    m_tops.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapTopsCollection::updateVisualization()
{
    auto polygonCollection = RimTools::polygonCollection();
    if ( !polygonCollection ) return;

    polygonCollection->updateAllRequiredEditors();

    if ( auto* project = RimProject::current() )
    {
        for ( auto* view : project->allViews() )
        {
            if ( auto* gridView = dynamic_cast<RimGridView*>( view ) )
            {
                if ( auto* polyCollection = gridView->polygonInViewCollection() )
                {
                    polyCollection->updateFromPolygonCollection();
                    polyCollection->updateConnectedEditors();
                }
                gridView->scheduleCreateDisplayModelAndRedraw();
            }
        }
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimContourMapTop*> RimContourMapTopsCollection::tops() const
{
    return m_tops.childrenByType();
}

//--------------------------------------------------------------------------------------------------
/// Deletes the existing tops and recomputes new ones for the owning contour map projection, using
/// the current topCount/minDistance field values.
//--------------------------------------------------------------------------------------------------
void RimContourMapTopsCollection::computeTops()
{
    auto* contourMapProjection = firstAncestorOrThisOfType<RimContourMapProjection>();
    if ( !contourMapProjection ) return;

    auto rigContourMapProjection = contourMapProjection->mapProjection();
    if ( !rigContourMapProjection ) return;

    RigContourMapTopFinder::Settings settings;
    settings.maxTops         = m_topCount;
    settings.minDistance     = m_minDistance;
    settings.excludeEdgeTops = true;

    auto tops = rigContourMapProjection->findTops( settings );

    // Rank the detected tops by value (highest first), independent of the prominence-based criterion
    // used to select which peaks to keep.
    std::sort( tops.begin(), tops.end(), []( const auto& a, const auto& b ) { return a.z > b.z; } );

    clearTops();

    auto origin3d = rigContourMapProjection->origin3d();
    auto depth    = rigContourMapProjection->topDepthBoundingBox();

    // Sphere radius factor is multiplied by the view's characteristic cell size when rendered.
    const double sphereRadiusFactor = 0.3;

    for ( size_t i = 0; i < tops.size(); ++i )
    {
        const auto& top = tops[i];

        cvf::Vec3d domainPoint( origin3d.x() + top.x, origin3d.y() + top.y, depth );
        addTop( static_cast<int>( i + 1 ), top.z, top.prominence, domainPoint, sphereRadiusFactor );
    }

    updateVisualization();

    Riu3DMainWindowTools::selectAsCurrentItem( this );
    Riu3DMainWindowTools::setExpanded( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapTopsCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_topCount );
    uiOrdering.add( &m_minDistance );
    uiOrdering.addNewButton( "Compute", [this]() { computeTops(); } );

    uiOrdering.skipRemainingFields();
}
