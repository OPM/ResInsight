/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024 Equinor ASA
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

#include "RimWellPathImport.h"

#include "RimFileWellPath.h"
#include "RimOilFieldEntry.h"
#include "RimOilRegionEntry.h"
#include "RimTools.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiTreeAttributes.h"
#include "cafPdmUiTreeViewEditor.h"

#include <QFileInfo>

namespace caf
{
template <>
void caf::AppEnum<RimWellPathImport::UtmFilterEnum>::setUp()
{
    addItem( RimWellPathImport::UTM_FILTER_OFF, "UTM_FILTER_OFF", "Off" );
    addItem( RimWellPathImport::UTM_FILTER_PROJECT, "UTM_FILTER_PROJECT", "Project" );
    addItem( RimWellPathImport::UTM_FILTER_CUSTOM, "UTM_FILTER_CUSTOM", "Custom" );
    setDefault( RimWellPathImport::UTM_FILTER_PROJECT );
}

} // End namespace caf

CAF_PDM_SOURCE_INIT( RimWellPathImport, "RimWellPathImport" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathImport::RimWellPathImport()
{
    CAF_PDM_InitObject( "RimWellPathImport" );

    caf::AppEnum<RimWellPathImport::UtmFilterEnum> defaultUtmMode = UTM_FILTER_OFF;
    CAF_PDM_InitField( &utmFilterMode, "UtmMode", defaultUtmMode, "Utm Filter" );

    CAF_PDM_InitField( &north, "UtmNorth", 0.0, "North" );
    CAF_PDM_InitField( &south, "UtmSouth", 0.0, "South" );
    CAF_PDM_InitField( &east, "UtmEast", 0.0, "East" );
    CAF_PDM_InitField( &west, "UtmWest", 0.0, "West" );

    CAF_PDM_InitFieldNoDefault( &regions_OBSOLETE, "Regions", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathImport::initAfterRead()
{
    updateFieldVisibility();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathImport::updateFieldVisibility()
{
    if ( utmFilterMode == UTM_FILTER_CUSTOM )
    {
        north.uiCapability()->setUiReadOnly( false );
        south.uiCapability()->setUiReadOnly( false );
        east.uiCapability()->setUiReadOnly( false );
        west.uiCapability()->setUiReadOnly( false );
    }
    else
    {
        north.uiCapability()->setUiReadOnly( true );
        south.uiCapability()->setUiReadOnly( true );
        east.uiCapability()->setUiReadOnly( true );
        west.uiCapability()->setUiReadOnly( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathImport::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &utmFilterMode )
    {
        updateFieldVisibility();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathImport::~RimWellPathImport()
{
    regions_OBSOLETE.deleteChildren();
}
