/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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
#include "RimWorkflowCollection.h"

#include "RimWorkflowInstalledCollection.h"
#include "RimWorkflowTaskCatalog.h"

#include "RiaLogging.h"
#include "RiaPreferencesSystem.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiTreeOrdering.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonArray>

CAF_PDM_SOURCE_INIT( RimWorkflowCollection, "WorkflowCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflowCollection::RimWorkflowCollection()
{
    CAF_PDM_InitObject( "Workflows", ":/Folder.png" );
    CAF_PDM_InitFieldNoDefault( &m_items, "Workflows", "" );

    CAF_PDM_InitFieldNoDefault( &m_installed, "Installed", "" );
    m_installed = new RimWorkflowInstalledCollection;

    rescanWorkflows();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflowCollection::~RimWorkflowCollection() = default;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWorkflowCollection::discoveryDirectory()
{
    return QDir::homePath() + "/.taskmaestro/workflows";
}

//--------------------------------------------------------------------------------------------------
/// Reload the task catalog, the workflow folders and the installed workflows. Workflows that
/// have never been saved are kept.
//--------------------------------------------------------------------------------------------------
void RimWorkflowCollection::rescanWorkflows()
{
    for ( RimWorkflow* workflow : items() )
    {
        if ( workflow->source() != RimWorkflow::Source::Unsaved ) deleteItem( workflow );
    }
    m_installed->deleteAllItems();

    if ( !RiaPreferencesSystem::current()->isFeatureEnabled( "workflows" ) ) return;

    RimWorkflowTaskCatalog::invalidate();
    const RimWorkflowTaskCatalog& catalog = RimWorkflowTaskCatalog::instance();
    if ( !catalog.isValid() ) RiaLogging::warning( QString( "Workflow task catalog: %1" ).arg( catalog.loadError() ).toStdString() );
    for ( const QString& pluginError : catalog.pluginErrors() )
        RiaLogging::warning( QString( "Workflow plugin: %1" ).arg( pluginError ).toStdString() );

    QDir dir( discoveryDirectory() );
    if ( dir.exists() )
    {
        const QFileInfoList entries = dir.entryInfoList( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );
        for ( const QFileInfo& entry : entries )
        {
            if ( !QFileInfo( entry.absoluteFilePath() + "/workflow.yaml" ).isFile() ) continue;

            auto* workflow = new RimWorkflow;
            workflow->setWorkflowDirectory( entry.absoluteFilePath() );
            workflow->loadFromDirectory();
            addItem( workflow );
        }
    }

    for ( const QJsonValue& entry : catalog.registeredWorkflows() )
    {
        auto* workflow = new RimWorkflow;
        workflow->loadFromRegistered( entry.toObject() );
        m_installed->addItem( workflow );
    }

    uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWorkflowCollection::hasUnsavedChanges() const
{
    for ( RimWorkflow* workflow : items() )
    {
        if ( workflow->isEditable() && workflow->isDirty() ) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWorkflow*> RimWorkflowCollection::allWorkflows() const
{
    std::vector<RimWorkflow*> workflows = items();
    for ( RimWorkflow* workflow : m_installed->items() )
        workflows.push_back( workflow );
    return workflows;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflow* RimWorkflowCollection::findWorkflowByDirectory( const QString& directory ) const
{
    const QString wanted = QDir( directory ).absolutePath();
    for ( RimWorkflow* workflow : items() )
    {
        if ( !workflow->workflowDirectory().isEmpty() && QDir( workflow->workflowDirectory() ).absolutePath() == wanted ) return workflow;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflowCollection::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicNewWorkflowFeature";
    menuBuilder << "RicRescanWorkflowsFeature";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflowCollection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    for ( RimWorkflow* workflow : items() )
        uiTreeOrdering.add( workflow );
    if ( m_installed->count() > 0 ) uiTreeOrdering.add( m_installed() );
    uiTreeOrdering.skipRemainingChildren( true );
}
