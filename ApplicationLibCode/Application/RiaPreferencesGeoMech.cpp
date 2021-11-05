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
    CAF_PDM_InitFieldNoDefault( &m_geomechFRAPreprocCommand, "geomechFRAPreprocCommand", "Pre-Processing Command" );
    m_geomechFRAPreprocCommand.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    m_geomechFRAPreprocCommand.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    CAF_PDM_InitFieldNoDefault( &m_geomechFRAPostprocCommand, "geomechFRAPostprocCommand", "Post-Processing Command" );
    m_geomechFRAPostprocCommand.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    m_geomechFRAPostprocCommand.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    CAF_PDM_InitFieldNoDefault( &m_geomechFRAMacrisCommand, "geomechFRAMacrisCommand", "Main Macris Command" );
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

    CAF_PDM_InitFieldNoDefault( &m_geomechWIADefaultXML, "geomechWIADefaultXML", "Default Parameter XML File" );
    m_geomechWIADefaultXML.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    m_geomechWIADefaultXML.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    CAF_PDM_InitFieldNoDefault( &m_geomechWIACommand, "geomechWIACommand", "Command to run" );
    m_geomechWIACommand.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    m_geomechWIACommand.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

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
    caf::PdmUiGroup* cmdFRAGroup  = faultRAGroup->addNewGroup( "Commands (without parameters)" );

    cmdFRAGroup->add( &m_geomechFRAPreprocCommand );
    cmdFRAGroup->add( &m_geomechFRAPostprocCommand );
    cmdFRAGroup->add( &m_geomechFRAMacrisCommand );

    caf::PdmUiGroup* paramFRAGroup = faultRAGroup->addNewGroup( "Parameters" );
    paramFRAGroup->add( &m_geomechFRADefaultBasicXML );
    paramFRAGroup->add( &m_geomechFRADefaultAdvXML );

    caf::PdmUiGroup* wellIAGroup = uiOrdering.addNewGroup( "Well Integrity Analysis" );
    wellIAGroup->add( &m_geomechWIACommand );
    wellIAGroup->add( &m_geomechWIADefaultXML );

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
bool RiaPreferencesGeoMech::validateFRASettings() const
{
    QStringList files;
    files << geomechFRAPreprocCommand();
    files << geomechFRAPostprocCommand();
    files << geomechFRAMacrisCommand();
    files << geomechFRADefaultBasicXML();
    files << geomechFRADefaultAdvXML();

    return filesExists( files );
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
