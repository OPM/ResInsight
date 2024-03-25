/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimOilFieldEntry.h"
#include "RimWellPathImport.h"

#include "RifJsonEncodeDecode.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMap>

CAF_PDM_SOURCE_INIT( RimOilFieldEntry, "RimOilFieldEntry" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimOilFieldEntry::RimOilFieldEntry()
{
    CAF_PDM_InitObject( "OilFieldEntry" );

    CAF_PDM_InitFieldNoDefault( &name, "OilFieldName", "Oil Field Name" );
    CAF_PDM_InitFieldNoDefault( &edmId, "EdmId", "Edm ID" );
    CAF_PDM_InitField( &selected, "Selected", false, "Selected" );

    CAF_PDM_InitFieldNoDefault( &wellsFilePath, "wellsFilePath", "Wells File Path" );

    CAF_PDM_InitFieldNoDefault( &wells, "Wells", "" );
    wells.uiCapability()->setUiTreeChildrenHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimOilFieldEntry::~RimOilFieldEntry()
{
    wells.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimOilFieldEntry::userDescriptionField()
{
    return &name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimOilFieldEntry::objectToggleField()
{
    return &selected;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOilFieldEntry::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &selected )
    {
        updateEnabledState();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOilFieldEntry::initAfterRead()
{
    updateEnabledState();

    updateUiIconFromToggleField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOilFieldEntry::updateEnabledState()
{
    bool wellsReadOnly = !selected;
    if ( isUiReadOnly() )
    {
        wellsReadOnly = true;
    }

    for ( size_t i = 0; i < wells.size(); i++ )
    {
        wells[i]->setUiReadOnly( wellsReadOnly );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathEntry* RimOilFieldEntry::find( const QString& entryName, RimWellPathEntry::WellTypeEnum wellPathType )
{
    for ( size_t i = 0; i < wells.size(); i++ )
    {
        RimWellPathEntry* wellPathEntry = wells[i];
        if ( wellPathEntry->name == entryName && wellPathEntry->wellPathType == wellPathType )
        {
            return wellPathEntry;
        }
    }

    return nullptr;
}
