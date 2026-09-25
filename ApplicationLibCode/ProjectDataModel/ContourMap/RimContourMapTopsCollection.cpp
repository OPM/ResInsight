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

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonCollection.h"
#include "Polygons/RimPolygonInViewCollection.h"

#include "RimGridView.h"
#include "RimProject.h"
#include "RimTools.h"

CAF_PDM_SOURCE_INIT( RimContourMapTopsCollection, "RimContourMapTopsCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimContourMapTopsCollection::RimContourMapTopsCollection()
{
    CAF_PDM_InitObject( "Detected Tops", ":/WellTargetPoint16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_tops, "Tops", "Tops" );

    nameField()->uiCapability()->setUiReadOnly( true );
    setName( "Detected Tops" );
}

//--------------------------------------------------------------------------------------------------
/// Creates both the RimContourMapTop metadata object and its diamond-shaped marker polygon (used to
/// visualize the top's position, reusing the existing polygon rendering/mirroring machinery).
//--------------------------------------------------------------------------------------------------
RimContourMapTop*
    RimContourMapTopsCollection::addTop( int rank, double value, double prominence, const cvf::Vec3d& domainPosition, double markerSize )
{
    auto polygonCollection = RimTools::polygonCollection();
    if ( !polygonCollection ) return nullptr;

    auto* top = new RimContourMapTop;
    top->setValues( rank, value, prominence, domainPosition );

    std::vector<cvf::Vec3d> diamond = { domainPosition + cvf::Vec3d( 0.0, markerSize, 0.0 ),
                                        domainPosition + cvf::Vec3d( markerSize, 0.0, 0.0 ),
                                        domainPosition + cvf::Vec3d( 0.0, -markerSize, 0.0 ),
                                        domainPosition + cvf::Vec3d( -markerSize, 0.0, 0.0 ) };

    auto* markerPolygon = polygonCollection->appendUserDefinedPolygon();
    markerPolygon->setPointsInDomainCoords( diamond );
    markerPolygon->setName( top->name() );
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
