/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RimGeoMechPart.h"

#include "RimGeoMechView.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimGeoMechPart, "GeoMechPart" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechPart::RimGeoMechPart()
{
    CAF_PDM_InitScriptableObject( "GeoMechPart", ":/GeoMechCase24x24.png", "", "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_partId, "PartId", "Part Id", "", "", "" );
    m_partId.uiCapability()->setUiReadOnly( true );

    nameField()->uiCapability()->setUiReadOnly( true );

    setDeletable( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechPart::~RimGeoMechPart()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechPart::setPartId( int partId )
{
    m_partId = partId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimGeoMechPart::partId() const
{
    return m_partId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechPart::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                       const QVariant&            oldValue,
                                       const QVariant&            newValue )
{
    updateUiIconFromToggleField();

    if ( changedField == objectToggleField() )
    {
        RimGeoMechView* ownerView;
        firstAncestorOrThisOfType( ownerView );
        if ( ownerView ) ownerView->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechPart::setDisplacements( const std::vector<cvf::Vec3f>& displacements )
{
    m_displacements = displacements;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::Vec3f> RimGeoMechPart::displacements() const
{
    return m_displacements;
}
