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

#include "RimContourMapPeak.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonCollection.h"

CAF_PDM_SOURCE_INIT( RimContourMapPeak, "RimContourMapPeak" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimContourMapPeak::RimContourMapPeak()
{
    CAF_PDM_InitObject( "Peak", ":/WellTargetPoint16x16.png" );

    CAF_PDM_InitField( &m_rank, "Rank", 0, "Rank" );
    m_rank.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_value, "Value", 0.0, "Value" );
    m_value.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_prominence, "Prominence", 0.0, "Prominence" );
    m_prominence.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_position, "Position", cvf::Vec3d::ZERO, "Position" );
    m_position.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_markerPolygon, "MarkerPolygon", "Marker Polygon" );
    m_markerPolygon.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
/// Delete the marker polygon along with this object, so no orphaned polygons are left behind in the
/// project-wide polygon collection when a peak is removed (e.g. via the generic delete-item command).
//--------------------------------------------------------------------------------------------------
RimContourMapPeak::~RimContourMapPeak()
{
    deleteMarkerPolygon();
}

//--------------------------------------------------------------------------------------------------
/// Only the marker polygon and its direct parent field are accessed, no project-wide lookups, so this is
/// safe during project teardown. If the polygon collection is destroyed first, the pointer field is
/// already null. The polygon is removed from whichever container owns it, also if the user moved it into
/// a sub-folder. The project tree and views are refreshed later from the event loop, as this is also
/// called when a whole contour map view is deleted.
//--------------------------------------------------------------------------------------------------
void RimContourMapPeak::deleteMarkerPolygon()
{
    RimPolygon* polygon = m_markerPolygon();
    if ( !polygon ) return;

    m_markerPolygon = nullptr;

    if ( auto* parentField = polygon->parentField() ) parentField->removeChild( polygon );
    delete polygon;

    RimPolygonCollection::scheduleUpdateViewsAfterPolygonsChanged();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapPeak::setValues( int rank, double value, double prominence, const cvf::Vec3d& domainPosition )
{
    m_rank       = rank;
    m_value      = value;
    m_prominence = prominence;
    m_position   = domainPosition;

    setName( QString( "Peak %1 (%2)" ).arg( rank ).arg( value, 0, 'g', 6 ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapPeak::setMarkerPolygon( RimPolygon* markerPolygon )
{
    m_markerPolygon = markerPolygon;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapPeak::releaseMarkerPolygon()
{
    m_markerPolygon = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimContourMapPeak::rank() const
{
    return m_rank();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapPeak::value() const
{
    return m_value();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapPeak::prominence() const
{
    return m_prominence();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimContourMapPeak::position() const
{
    return m_position();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygon* RimContourMapPeak::markerPolygon() const
{
    return m_markerPolygon();
}
