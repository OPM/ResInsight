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
    CAF_PDM_InitFieldNoDefault( &m_geomechFRAPreprocCommand, "geomechFRAPreprocCommand", "Pre-Processing Command", "", "", "" );
    m_geomechFRAPreprocCommand.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    m_geomechFRAPreprocCommand.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    CAF_PDM_InitFieldNoDefault( &m_geomechFRAPostprocCommand, "geomechFRAPostprocCommand", "Post-Processing Command", "", "", "" );
    m_geomechFRAPostprocCommand.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    m_geomechFRAPostprocCommand.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    CAF_PDM_InitFieldNoDefault( &m_geomechFRAMacrisCommand, "geomechFRAMacrisCommand", "Main Macris Command", "", "", "" );
    m_geomechFRAMacrisCommand.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    m_geomechFRAMacrisCommand.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    CAF_PDM_InitFieldNoDefault( &m_geomechFRADefaultBasicXML,
                                "geomechFRADefaultXML",
                                "Basic Processing Parameter XML File",
                                "",
                                "",
                                "" );
    m_geomechFRADefaultBasicXML.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    m_geomechFRADefaultBasicXML.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    CAF_PDM_InitFieldNoDefault( &m_geomechFRADefaultAdvXML,
                                "geomechFRADefaultAdvXML",
                                "Advanced Processing Parameter XML File",
                                "",
                                "",
                                "" );
    m_geomechFRADefaultAdvXML.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    m_geomechFRADefaultAdvXML.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    CAF_PDM_InitField( &m_keepTemporaryFiles,
                       "keepTemporaryFile",
                       false,
                       "Keep temporary parameter files (for debugging)",
                       "",
                       "",
                       "" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_keepTemporaryFiles );
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
    caf::PdmUiGroup* faultRAGroup = uiOrdering.addNewGroup( "Fault Reactivation Assessment" );
    caf::PdmUiGroup* cmdGroup     = faultRAGroup->addNewGroup( "Commands (without parameters)" );

    cmdGroup->add( &m_geomechFRAPreprocCommand );
    cmdGroup->add( &m_geomechFRAPostprocCommand );
    cmdGroup->add( &m_geomechFRAMacrisCommand );

    caf::PdmUiGroup* paramGroup = faultRAGroup->addNewGroup( "Parameters" );
    paramGroup->add( &m_geomechFRADefaultBasicXML );
    paramGroup->add( &m_geomechFRADefaultAdvXML );

    caf::PdmUiGroup* settingsGroup = faultRAGroup->addNewGroup( "Settings" );
    settingsGroup->add( &m_keepTemporaryFiles );
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
QString RiaPreferencesGeoMech::geomechFRAPreprocCommand() const
{
    return m_geomechFRAPreprocCommand;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferencesGeoMech::geomechFRAPostprocCommand() const
{
    return m_geomechFRAPostprocCommand;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferencesGeoMech::geomechFRAMacrisCommand() const
{
    return m_geomechFRAMacrisCommand;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferencesGeoMech::geomechFRADefaultBasicXML() const
{
    return m_geomechFRADefaultBasicXML;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferencesGeoMech::geomechFRADefaultAdvXML() const
{
    return m_geomechFRADefaultAdvXML;
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
bool RiaPreferencesGeoMech::validateFRASettings() const
{
    QStringList files;
    files << geomechFRAPreprocCommand();
    files << geomechFRAPostprocCommand();
    files << geomechFRAMacrisCommand();
    files << geomechFRADefaultBasicXML();
    files << geomechFRADefaultAdvXML();

    for ( int i = 0; i < files.size(); i++ )
    {
        if ( files[i].isEmpty() ) return false;
        QFile file( files[i] );
        if ( !file.exists() ) return false;
    }

    return true;
}
