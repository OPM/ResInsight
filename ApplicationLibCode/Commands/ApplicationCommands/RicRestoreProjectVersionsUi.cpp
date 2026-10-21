/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RicRestoreProjectVersionsUi.h"

#include "RiuRecentFileActionProvider.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiOrdering.h"

#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RicRestoreProjectVersionsUi, "RicRestoreProjectVersionsUi" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicRestoreProjectVersionsUi::RicRestoreProjectVersionsUi()
{
    CAF_PDM_InitObject( "Restore Project Versions" );

    CAF_PDM_InitFieldNoDefault( &m_recentProjectFiles, "RecentProjectFiles", "Recent Project Files" );
    m_recentProjectFiles.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_projectFile, "ProjectFile", "Project File" );
    m_projectFile.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    m_projectFile.uiCapability()->setAttribute( caf::PdmUiFilePathEditor::Keys::FILE_SELECTION_FILTER,
                                                QString( "ResInsight project (*.rsp *.rip);;All files (*.*)" ) );

    CAF_PDM_InitFieldNoDefault( &m_exportFolder, "ExportFolder", "Export Folder" );
    m_exportFolder.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    m_exportFolder.uiCapability()->setAttribute( caf::PdmUiFilePathEditor::Keys::SELECT_DIRECTORY, true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicRestoreProjectVersionsUi::projectFile() const
{
    return m_projectFile().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicRestoreProjectVersionsUi::exportFolder() const
{
    return m_exportFolder().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRestoreProjectVersionsUi::setProjectFile( const QString& filePath )
{
    m_projectFile = caf::FilePath( filePath );

    // Keep the recent files combo box in sync so it reflects the currently selected project file
    // instead of appearing empty when a default/current project file is set programmatically.
    QStringList recentFiles = RiuRecentFileActionProvider::recentProjectFiles();
    m_recentProjectFiles    = recentFiles.contains( filePath ) ? filePath : QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRestoreProjectVersionsUi::setExportFolder( const QString& folderPath )
{
    m_exportFolder = caf::FilePath( folderPath );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RicRestoreProjectVersionsUi::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_recentProjectFiles )
    {
        for ( const QString& recentFile : RiuRecentFileActionProvider::recentProjectFiles() )
        {
            QString uiText = QFileInfo( recentFile ).fileName() + "  (" + recentFile + ")";
            options.push_back( caf::PdmOptionItemInfo( uiText, recentFile ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRestoreProjectVersionsUi::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_recentProjectFiles )
    {
        QString selectedFile = m_recentProjectFiles();
        if ( !selectedFile.isEmpty() )
        {
            setProjectFile( selectedFile );
            setExportFolder( QFileInfo( selectedFile ).absolutePath() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRestoreProjectVersionsUi::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    bool hasRecentFiles = !RiuRecentFileActionProvider::recentProjectFiles().isEmpty();
    m_recentProjectFiles.uiCapability()->setUiHidden( !hasRecentFiles );

    auto infoGroup = uiOrdering.addNewGroup( "Information" );
    infoGroup->addNewLabel( "Restores previous revisions of a ResInsight project from its backup history. Select the "
                            "project file to restore from and an export folder, and each stored revision will be "
                            "written as a separate project file." );

    auto selectionGroup = uiOrdering.addNewGroup( "Project Selection" );
    selectionGroup->add( &m_recentProjectFiles );
    selectionGroup->add( &m_projectFile );
    selectionGroup->add( &m_exportFolder );
}
