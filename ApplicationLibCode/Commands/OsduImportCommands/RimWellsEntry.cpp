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

#include "RimWellsEntry.h"
#include "cafAppEnum.h"

namespace caf
{
template <>
void caf::AppEnum<RimWellPathEntry::WellTypeEnum>::setUp()
{
    addItem( RimWellPathEntry::WELL_ALL, "WELL_ALL", "All" );
    addItem( RimWellPathEntry::WELL_SURVEY, "WELL_SURVEY", "Survey" );
    addItem( RimWellPathEntry::WELL_PLAN, "WELL_PLAN", "Plan" );
    setDefault( RimWellPathEntry::WELL_ALL );
}

} // End namespace caf

CAF_PDM_SOURCE_INIT( RimWellPathEntry, "RimWellPathEntry" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathEntry::RimWellPathEntry()
{
    CAF_PDM_InitObject( "WellPathEntry" );

    CAF_PDM_InitFieldNoDefault( &name, "Name", "Name" );
    CAF_PDM_InitField( &selected, "Selected", false, "Selected" );

    caf::AppEnum<RimWellPathEntry::WellTypeEnum> wellType = WELL_ALL;
    CAF_PDM_InitField( &wellPathType, "WellPathType", wellType, "Well path type" );

    CAF_PDM_InitFieldNoDefault( &surveyType, "surveyType", "surveyType" );
    CAF_PDM_InitFieldNoDefault( &requestUrl, "requestUrl", "requestUrl" );

    CAF_PDM_InitFieldNoDefault( &wellPathFilePath, "wellPathFilePath", "wellPathFilePath" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellPathEntry::userDescriptionField()
{
    return &name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellPathEntry::objectToggleField()
{
    return &selected;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathEntry* RimWellPathEntry::createWellPathEntry( const QString& name,
                                                         const QString& surveyType,
                                                         const QString& requestUrl,
                                                         const QString& absolutePath,
                                                         WellTypeEnum   wellType )
{
    RimWellPathEntry* entry = new RimWellPathEntry();
    entry->name             = name;
    entry->surveyType       = surveyType;
    entry->requestUrl       = requestUrl;
    entry->wellPathType     = wellType;

    QString responseFilePath = undefinedWellPathLocation();

    int strIndex = requestUrl.indexOf( "edm/" );
    if ( strIndex >= 0 )
    {
        QString lastPart = requestUrl.right( requestUrl.size() - strIndex );
        lastPart         = lastPart.replace( '/', '_' );

        responseFilePath = absolutePath + "/" + lastPart + ".json";
    }

    entry->wellPathFilePath = responseFilePath;

    return entry;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathEntry::undefinedWellPathLocation()
{
    return "UNDEFIED_WELLPATH_FILE";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPathEntry::isWellPathValid()
{
    return ( wellPathFilePath().compare( undefinedWellPathLocation() ) != 0 );
}
