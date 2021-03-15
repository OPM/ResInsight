/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 -     Equinor ASA
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

#include "RimFractureGroupStatistics.h"

#include "cafPdmUiTextEditor.h"

CAF_PDM_SOURCE_INIT( RimFractureGroupStatistics, "FractureGroupStatistics" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureGroupStatistics::RimFractureGroupStatistics()
{
    CAF_PDM_InitObject( "Fracture Group Statistics", ":/FractureTemplate16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_filePaths, "FilePaths", "", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_filePathsTable, "FilePathsTable", "File Paths Table", "", "", "" );
    m_filePathsTable.uiCapability()->setUiEditorTypeName( caf::PdmUiTextEditor::uiEditorTypeName() );
    m_filePathsTable.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_filePathsTable.uiCapability()->setUiReadOnly( true );
    m_filePathsTable.xmlCapability()->disableIO();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureGroupStatistics::~RimFractureGroupStatistics()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureGroupStatistics::addFilePath( const QString& filePath )
{
    m_filePaths.v().push_back( filePath );
    m_filePathsTable = generateFilePathsTable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFractureGroupStatistics::generateFilePathsTable()
{
    QString body;
    for ( auto prop : m_filePaths.v() )
    {
        body.append( prop.path() + "<br>" );
    }

    return body;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureGroupStatistics::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                        QString                    uiConfigName,
                                                        caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_filePathsTable )
    {
        auto myAttr = dynamic_cast<caf::PdmUiTextEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->wrapMode = caf::PdmUiTextEditorAttribute::NoWrap;
            myAttr->textMode = caf::PdmUiTextEditorAttribute::HTML;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureGroupStatistics::loadAndUpdateData()
{
    m_filePathsTable = generateFilePathsTable();
}
