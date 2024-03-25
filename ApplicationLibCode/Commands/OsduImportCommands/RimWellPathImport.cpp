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

    CAF_PDM_InitFieldNoDefault( &regions, "Regions", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathImport::updateRegions( const QStringList& regionStrings, const QStringList& fieldStrings, const QStringList& edmIds )
{
    assert( regionStrings.size() == fieldStrings.size() && regionStrings.size() == edmIds.size() );

    std::vector<RimOilRegionEntry*> regionsToRemove;

    // Remove regions and fields not present in last request
    for ( size_t regionIdx = 0; regionIdx < regions.size(); regionIdx++ )
    {
        if ( !regionStrings.contains( regions[regionIdx]->name ) )
        {
            regionsToRemove.push_back( regions[regionIdx] );
        }
        else
        {
            std::vector<RimOilFieldEntry*> fieldsToRemove;

            for ( size_t fIdx = 0; fIdx < regions[regionIdx]->fields.size(); fIdx++ )
            {
                if ( !fieldStrings.contains( regions[regionIdx]->fields[fIdx]->name ) )
                {
                    fieldsToRemove.push_back( regions[regionIdx]->fields[fIdx] );
                }
            }

            for ( size_t i = 0; i < fieldsToRemove.size(); i++ )
            {
                regions[regionIdx]->fields.removeChild( fieldsToRemove[i] );

                delete fieldsToRemove[i];
            }
        }
    }

    for ( size_t i = 0; i < regionsToRemove.size(); i++ )
    {
        regions.removeChild( regionsToRemove[i] );

        delete regionsToRemove[i];
    }

    for ( int i = 0; i < regionStrings.size(); i++ )
    {
        RimOilRegionEntry* oilRegionEntry = nullptr;
        RimOilFieldEntry*  oilFieldEntry  = nullptr;

        for ( size_t regionIdx = 0; regionIdx < regions.size(); regionIdx++ )
        {
            if ( regions[regionIdx]->name == regionStrings[i] )
            {
                oilRegionEntry = regions[regionIdx];

                for ( size_t fIdx = 0; fIdx < regions[regionIdx]->fields.size(); fIdx++ )
                {
                    if ( regions[regionIdx]->fields[fIdx]->edmId == edmIds[i] )
                    {
                        oilFieldEntry = regions[regionIdx]->fields[fIdx];
                    }
                }
            }
        }

        if ( !oilRegionEntry )
        {
            oilRegionEntry       = new RimOilRegionEntry;
            oilRegionEntry->name = regionStrings[i];

            regions.push_back( oilRegionEntry );
        }

        assert( oilRegionEntry );

        if ( !oilFieldEntry )
        {
            oilFieldEntry        = new RimOilFieldEntry;
            oilFieldEntry->name  = fieldStrings[i];
            oilFieldEntry->edmId = edmIds[i];

            oilRegionEntry->fields.push_back( oilFieldEntry );
        }
    }

    updateFieldVisibility();
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
void RimWellPathImport::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    caf::PdmUiTreeViewEditorAttribute* myAttr = dynamic_cast<caf::PdmUiTreeViewEditorAttribute*>( attribute );
    if ( myAttr )
    {
        QStringList colHeaders;
        colHeaders << "Region";
        myAttr->columnHeaders = colHeaders;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathImport::~RimWellPathImport()
{
    regions.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathImport::updateFilePaths()
{
    QString wellPathsFolderPath = RimFileWellPath::getCacheDirectoryPath();

    for ( size_t regionIdx = 0; regionIdx < regions.size(); regionIdx++ )
    {
        for ( size_t fIdx = 0; fIdx < regions[regionIdx]->fields.size(); fIdx++ )
        {
            RimOilFieldEntry* oilField = regions[regionIdx]->fields[fIdx];

            QFileInfo fi( oilField->wellsFilePath );

            QString newWellsFilePath = wellPathsFolderPath + "/" + fi.fileName();
            oilField->wellsFilePath  = newWellsFilePath;

            for ( size_t wIdx = 0; wIdx < oilField->wells.size(); wIdx++ )
            {
                RimWellPathEntry* rimWellPathEntry = oilField->wells[wIdx];

                QFileInfo fiWell( rimWellPathEntry->wellPathFilePath );

                QString newFilePath                = wellPathsFolderPath + "/" + fiWell.fileName();
                rimWellPathEntry->wellPathFilePath = newFilePath;
            }
        }
    }
}
