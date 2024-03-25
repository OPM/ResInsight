/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RimOilRegionEntry.h"
#include "RimOilFieldEntry.h"

CAF_PDM_SOURCE_INIT( RimOilRegionEntry, "RimOilRegionEntry" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimOilRegionEntry::RimOilRegionEntry()
{
    CAF_PDM_InitObject( "OilRegionEntry" );

    CAF_PDM_InitFieldNoDefault( &name, "OilRegionEntry", "OilRegionEntry" );

    CAF_PDM_InitFieldNoDefault( &fields, "Fields", "" );

    CAF_PDM_InitField( &selected, "Selected", false, "Selected" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimOilRegionEntry::~RimOilRegionEntry()
{
    fields.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimOilRegionEntry::userDescriptionField()
{
    return &name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimOilRegionEntry::objectToggleField()
{
    return &selected;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOilRegionEntry::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( &selected == changedField )
    {
        updateState();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOilRegionEntry::updateState()
{
    for ( size_t i = 0; i < fields.size(); i++ )
    {
        fields[i]->setUiReadOnly( !selected );
        fields[i]->updateEnabledState();
    }
}
