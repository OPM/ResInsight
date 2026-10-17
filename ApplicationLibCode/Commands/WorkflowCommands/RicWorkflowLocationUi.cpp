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

#include "RicWorkflowLocationUi.h"

#include "Workflow/RimWorkflowCollection.h"
#include "Workflow/RimWorkflowDefinitionTools.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiPropertyViewDialog.h"

#include <QDir>
#include <QMessageBox>

CAF_PDM_SOURCE_INIT( RicWorkflowLocationUi, "RicWorkflowLocationUi" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicWorkflowLocationUi::RicWorkflowLocationUi()
{
    CAF_PDM_InitObject( "Workflow Location" );

    CAF_PDM_InitFieldNoDefault( &m_name, "Name", "Name" );
    CAF_PDM_InitFieldNoDefault( &m_folder, "Folder", "Folder" );
    m_folder.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWorkflowLocationUi::setName( const QString& name )
{
    m_name = name;
    if ( !m_folderEditedByUser ) m_folder = defaultFolder( name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicWorkflowLocationUi::name() const
{
    return m_name().trimmed();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicWorkflowLocationUi::folder() const
{
    return m_folder().path().trimmed();
}

//--------------------------------------------------------------------------------------------------
/// A folder in the discovery folder named after the workflow
//--------------------------------------------------------------------------------------------------
QString RicWorkflowLocationUi::defaultFolder( const QString& name )
{
    return QDir( RimWorkflowCollection::discoveryDirectory() ).absoluteFilePath( RimWorkflowDefinitionTools::identifierFrom( name ) );
}

//--------------------------------------------------------------------------------------------------
/// Ask for a name and a folder. Returns nothing when cancelled. Warns before using a folder that
/// already holds a workflow, or a folder that will not be found by Rescan.
//--------------------------------------------------------------------------------------------------
std::optional<RicWorkflowLocationUi::Location> RicWorkflowLocationUi::askForLocation( const QString& title, const QString& defaultName )
{
    RicWorkflowLocationUi ui;
    ui.setName( defaultName );

    QWidget* parent = Riu3DMainWindowTools::mainWindowWidget();
    while ( true )
    {
        caf::PdmUiPropertyViewDialog dialog( parent, &ui, title, "", QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
        dialog.resize( QSize( 500, 150 ) );
        if ( dialog.exec() != QDialog::Accepted ) return std::nullopt;

        if ( ui.name().isEmpty() || ui.folder().isEmpty() )
        {
            QMessageBox::warning( parent, title, "Enter both a name and a folder." );
            continue;
        }

        const QDir folder( ui.folder() );
        if ( folder.exists( "workflow.yaml" ) )
        {
            const auto answer =
                QMessageBox::question( parent,
                                       title,
                                       QString( "'%1' already contains a workflow. Overwrite it when saving?" ).arg( ui.folder() ) );
            if ( answer != QMessageBox::Yes ) continue;
        }

        const QString discovery = QDir( RimWorkflowCollection::discoveryDirectory() ).absolutePath();
        if ( QDir( folder.absolutePath() + "/.." ).absolutePath() != discovery )
        {
            const auto answer = QMessageBox::question( parent,
                                                       title,
                                                       QString( "'%1' is not directly inside %2, so the workflow will not be listed "
                                                                "after Rescan or a restart. Use it anyway?" )
                                                           .arg( ui.folder(), discovery ) );
            if ( answer != QMessageBox::Yes ) continue;
        }

        return Location{ .name = ui.name(), .folder = folder.absolutePath() };
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWorkflowLocationUi::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_name )
    {
        if ( !m_folderEditedByUser ) m_folder = defaultFolder( m_name() );
    }
    else if ( changedField == &m_folder )
    {
        m_folderEditedByUser = true;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWorkflowLocationUi::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field != &m_folder ) return;
    if ( auto* attr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute ) )
    {
        attr->m_selectDirectory = true;
    }
}
