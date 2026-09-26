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

#include "RimContourMapTop.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonCollection.h"
#include "RimTools.h"

CAF_PDM_SOURCE_INIT( RimContourMapTop, "RimContourMapTop" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimContourMapTop::RimContourMapTop()
{
    CAF_PDM_InitObject( "Top", ":/WellTargetPoint16x16.png" );

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
/// project-wide polygon collection when a top is removed (e.g. via the generic delete-item command).
//--------------------------------------------------------------------------------------------------
RimContourMapTop::~RimContourMapTop()
{
    if ( auto* polygon = m_markerPolygon() )
    {
        if ( auto* collection = RimTools::polygonCollection() )
        {
            collection->deleteItem( polygon );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapTop::setValues( int rank, double value, double prominence, const cvf::Vec3d& domainPosition )
{
    m_rank       = rank;
    m_value      = value;
    m_prominence = prominence;
    m_position   = domainPosition;

    setName( QString( "Top %1 (%2)" ).arg( rank ).arg( value, 0, 'g', 6 ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapTop::setMarkerPolygon( RimPolygon* markerPolygon )
{
    m_markerPolygon = markerPolygon;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimContourMapTop::rank() const
{
    return m_rank();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapTop::value() const
{
    return m_value();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapTop::prominence() const
{
    return m_prominence();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimContourMapTop::position() const
{
    return m_position();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygon* RimContourMapTop::markerPolygon() const
{
    return m_markerPolygon();
}
