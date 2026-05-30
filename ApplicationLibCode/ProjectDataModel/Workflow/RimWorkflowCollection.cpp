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

#include <QDir>
#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RimWorkflowCollection, "WorkflowCollection" );

RimWorkflowCollection::RimWorkflowCollection()
{
    CAF_PDM_InitObject( "Workflows", ":/Folder.png" );
    CAF_PDM_InitFieldNoDefault( &m_items, "Workflows", "" );

    rescanWorkflows();
}

RimWorkflowCollection::~RimWorkflowCollection() = default;

QString RimWorkflowCollection::discoveryDirectory()
{
    return QDir::homePath() + "/.taskmaestro/workflows";
}

void RimWorkflowCollection::rescanWorkflows()
{
    deleteAllItems();

    QDir dir( discoveryDirectory() );
    if ( !dir.exists() ) return;

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
