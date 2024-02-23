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

#include "RiaApplication.h"
#include "Rim3dView.h"
#include "RimPolygonAppearance.h"
#include "RimPolygonTools.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeAttributes.h"

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

    CAF_PDM_InitField( &m_editPolygonButton, "EditPolygonButton", false, "Edit" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_editPolygonButton );

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
void RimPolygon::uiOrderingForLocalPolygon( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    m_appearance->uiOrdering( uiConfigName, uiOrdering );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygon::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicNewPolygonIntersectionFeature";
    menuBuilder << "RicNewPolygonFilterFeature";
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
void RimPolygon::setIsClosed( bool isClosed )
{
    m_appearance->setIsClosed( isClosed );
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
void RimPolygon::setReadOnly( bool isReadOnly )
{
    m_isReadOnly = isReadOnly;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolygon::isReadOnly() const
{
    return m_isReadOnly();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygon::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( nameField() );
    uiOrdering.add( &m_isReadOnly );
    uiOrdering.add( &m_editPolygonButton );

    auto groupPoints = uiOrdering.addNewGroup( "Points" );
    groupPoints->setCollapsedByDefault();
    groupPoints->add( &m_pointsInDomainCoords );

    m_pointsInDomainCoords.uiCapability()->setUiReadOnly( m_isReadOnly() );

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

    if ( changedField == &m_editPolygonButton )
    {
        auto activeView = RiaApplication::instance()->activeReservoirView();
        RimPolygonTools::selectAndActivatePolygonInView( this, activeView );

        m_editPolygonButton = false;

        return;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygon::childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField )
{
    objectChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygon::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_editPolygonButton )
    {
        if ( auto attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute ) )
        {
            attrib->m_buttonText = "Edit in Active View";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygon::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( m_isReadOnly )
    {
        caf::PdmUiTreeViewItemAttribute::createTagIfTreeViewItemAttribute( attribute, ":/padlock.svg" );
    }
}
