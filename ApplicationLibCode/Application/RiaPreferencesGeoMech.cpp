/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "RiaPreferencesGeoMech.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiFilePathEditor.h"

#include <QFile>
#include <QStringList>

CAF_PDM_SOURCE_INIT( RiaPreferencesGeoMech, "RiaPreferencesGeoMech" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesGeoMech::RiaPreferencesGeoMech()
{
    CAF_PDM_InitFieldNoDefault( &m_geomechWIADefaultXML, "geomechWIADefaultXML", "Default Parameter XML File" );
    m_geomechWIADefaultXML.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    m_geomechWIADefaultXML.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    CAF_PDM_InitFieldNoDefault( &m_geomechWIACommand, "geomechWIACommand", "Command to run" );
    m_geomechWIACommand.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    m_geomechWIACommand.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    CAF_PDM_InitField( &m_keepTemporaryFiles, "keepTemporaryFile", false, "Keep temporary parameter files (for debugging)" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_keepTemporaryFiles );

    CAF_PDM_InitField( &m_waitForInputFileEdit, "waitForInputFileEdit", true, "Pause to allow modification of input files before running modeling." );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_waitForInputFileEdit );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesGeoMech* RiaPreferencesGeoMech::current()
{
    return RiaApplication::instance()->preferences()->geoMechPreferences();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferencesGeoMech::appendItems( caf::PdmUiOrdering& uiOrdering ) const
{
    caf::PdmUiGroup* wellIAGroup = uiOrdering.addNewGroup( "Well Integrity Analysis" );
    wellIAGroup->add( &m_geomechWIACommand );
    wellIAGroup->add( &m_geomechWIADefaultXML );
    wellIAGroup->add( &m_waitForInputFileEdit );

    caf::PdmUiGroup* commonGroup = uiOrdering.addNewGroup( "Common Settings" );
    commonGroup->add( &m_keepTemporaryFiles );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferencesGeoMech::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferencesGeoMech::geomechWIACommand() const
{
    return m_geomechWIACommand;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferencesGeoMech::geomechWIADefaultXML() const
{
    return m_geomechWIADefaultXML;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesGeoMech::keepTemporaryFiles() const
{
    return m_keepTemporaryFiles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesGeoMech::waitBeforeRunWIA() const
{
    return m_waitForInputFileEdit;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesGeoMech::validateWIASettings() const
{
    QStringList files;
    files << geomechWIACommand();
    files << geomechWIADefaultXML();

    return filesExists( files );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesGeoMech::filesExists( QStringList& files ) const
{
    for ( int i = 0; i < files.size(); i++ )
    {
        if ( files[i].isEmpty() ) return false;
        QFile file( files[i] );
        if ( !file.exists() ) return false;
    }
    return true;
}
