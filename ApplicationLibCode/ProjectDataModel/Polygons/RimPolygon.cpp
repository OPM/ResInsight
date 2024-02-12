/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RimPolygon.h"

#include "RigPolyLinesData.h"
#include "RimPolygonAppearance.h"

CAF_PDM_SOURCE_INIT( RimPolygon, "RimPolygon" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygon::RimPolygon()
    : objectChanged( this )
{
    CAF_PDM_InitObject( "Polygon", ":/PolylinesFromFile16x16.png" );

    CAF_PDM_InitField( &m_isReadOnly, "IsReadOnly", false, "Read Only" );
    CAF_PDM_InitFieldNoDefault( &m_pointsInDomainCoords, "PointsInDomainCoords", "Points" );
    CAF_PDM_InitFieldNoDefault( &m_appearance, "Appearance", "Appearance" );
    m_appearance = new RimPolygonAppearance;
    m_appearance.uiCapability()->setUiTreeChildrenHidden( true );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigPolyLinesData> RimPolygon::polyLinesData() const
{
    cvf::ref<RigPolyLinesData> pld = new RigPolyLinesData;

    pld->setPolyLine( m_pointsInDomainCoords() );
    m_appearance->applyAppearanceSettings( pld.p() );

    return pld;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygon::setPointsInDomainCoords( const std::vector<cvf::Vec3d>& points )
{
    m_pointsInDomainCoords = points;

    objectChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimPolygon::pointsInDomainCoords() const
{
    return m_pointsInDomainCoords();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolygon::isClosed() const
{
    return m_appearance->isClosed();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygon::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( nameField() );
    uiOrdering.add( &m_isReadOnly );

    auto groupPoints = uiOrdering.addNewGroup( "Points" );
    groupPoints->setCollapsedByDefault();
    groupPoints->add( &m_pointsInDomainCoords );

    auto group = uiOrdering.addNewGroup( "Appearance" );
    m_appearance->uiOrdering( uiConfigName, *group );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygon::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_pointsInDomainCoords )
    {
        objectChanged.send();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygon::childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField )
{
    objectChanged.send();
}
