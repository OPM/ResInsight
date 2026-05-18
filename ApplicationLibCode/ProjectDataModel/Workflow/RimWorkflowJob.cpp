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

#include "RimWorkflowJob.h"

#include "RimWorkflow.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "RiuMainWindow.h"
#include "RiuWorkflowJobRunner.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiGroup.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmUiTreeOrdering.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QUuid>

CAF_PDM_SOURCE_INIT( RimWorkflowJob, "WorkflowJob" );

namespace
{
QString findPythonExecutable()
{
    QStringList candidates;
    if ( auto* prefs = RiaPreferences::current() )
    {
        QString configured = prefs->pythonExecutable();
        if ( !configured.isEmpty() && configured != "python" ) candidates << configured;
    }
    candidates << "python3" << "python";

    for ( const QString& cand : candidates )
    {
        if ( cand.contains( '/' ) || cand.contains( '\\' ) )
        {
            if ( QFileInfo( cand ).isExecutable() ) return cand;
        }
        else if ( !QStandardPaths::findExecutable( cand ).isEmpty() )
        {
            return cand;
        }
    }
    return {};
}
} // namespace

RimWorkflowJob::RimWorkflowJob()
{
    CAF_PDM_InitObject( "Job", ":/Bullet.png" );

    CAF_PDM_InitFieldNoDefault( &m_name, "Name", "Name" );

    CAF_PDM_InitFieldNoDefault( &m_taskInputs, "TaskInputs", "" );
}

void RimWorkflowJob::setJobName( const QString& name )
{
    m_name = name;
    setUiName( name );
}

void RimWorkflowJob::initAfterRead()
{
    setUiName( m_name() );
}

void RimWorkflowJob::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_name );

    for ( RimWorkflowTaskInput* task : m_taskInputs.childrenByType() )
    {
        if ( !task ) continue;
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( task->taskName() );
        for ( RimWorkflowFieldBinding* b : task->items() )
        {
            if ( b && b->valueField() ) group->add( b->valueField() );
        }
    }
    uiOrdering.skipRemainingFields( true );
}

void RimWorkflowJob::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    uiTreeOrdering.skipRemainingChildren( true );
}

std::vector<RimWorkflowTaskInput*> RimWorkflowJob::taskInputs() const
{
    std::vector<RimWorkflowTaskInput*> result;
    result.reserve( m_taskInputs.size() );
    for ( RimWorkflowTaskInput* t : m_taskInputs.childrenByType() )
    {
        if ( t ) result.push_back( t );
    }
    return result;
}

void RimWorkflowJob::setTaskInputs( std::vector<RimWorkflowTaskInput*> inputs )
{
    m_taskInputs.deleteChildren();
    for ( RimWorkflowTaskInput* t : inputs )
    {
        if ( t ) m_taskInputs.push_back( t );
    }
}

void RimWorkflowJob::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_name )
    {
        setUiName( m_name() );
        uiCapability()->updateConnectedEditors();
    }
}

void RimWorkflowJob::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicRunWorkflowJobFeature";
    menuBuilder << "RicCancelWorkflowJobFeature";
}

bool RimWorkflowJob::isRunning() const
{
    return m_runner && m_runner->isRunning();
}

void RimWorkflowJob::cancelJob()
{
    if ( m_runner ) m_runner->cancel();
}

QString RimWorkflowJob::writeInputYaml( const QString& path ) const
{
    QString body;
    for ( RimWorkflowTaskInput* t : m_taskInputs.childrenByType() )
    {
        if ( t ) body += t->toTaskYamlBlock();
    }

    QFile out( path );
    if ( !out.open( QIODevice::WriteOnly | QIODevice::Truncate ) ) return {};
    out.write( body.toUtf8() );
    out.close();
    return path;
}

void RimWorkflowJob::runJob()
{
    if ( isRunning() )
    {
        RiaLogging::warning( "Job is already running." );
        return;
    }

    auto* workflow = firstAncestorOrThisOfType<RimWorkflow>();
    if ( !workflow )
    {
        RiaLogging::warning( "Cannot run job: parent workflow not found." );
        return;
    }

    const QString dir = workflow->workflowDirectory();
    if ( dir.isEmpty() ) return;

    auto port = RiaApplication::instance()->activeGrpcPortNumber();
    if ( !port.has_value() )
    {
        RiaLogging::warning( "Cannot run workflow: gRPC server is not active. Enable it in Preferences." );
        return;
    }

    QString python = findPythonExecutable();
    if ( python.isEmpty() )
    {
        RiaLogging::warning( "Cannot run workflow: no Python interpreter found." );
        return;
    }

    QDir    tmp( QDir::tempPath() );
    QString runDir = QString( "resinsight_workflow_%1" ).arg( QUuid::createUuid().toString( QUuid::WithoutBraces ) );
    if ( !tmp.mkpath( runDir ) )
    {
        RiaLogging::warning( "Cannot create temp dir for workflow run." );
        return;
    }
    QString inputPath = tmp.absoluteFilePath( runDir + "/input.yaml" );
    if ( writeInputYaml( inputPath ).isEmpty() )
    {
        RiaLogging::warning( "Failed to write input.yaml" );
        return;
    }

    QStringList args{ "-m", "rips.taskmaestro_helper", "run", dir, "--input", inputPath, "--grpc-port", QString::number( port.value() ) };

    QProcessEnvironment env   = QProcessEnvironment::systemEnvironment();
    const QString       label = workflow->uiName() + " / " + m_name();

    m_runner = new RiuWorkflowJobRunner( label, RiuMainWindow::instance() );
    m_runner->start( python, args, env );
}
