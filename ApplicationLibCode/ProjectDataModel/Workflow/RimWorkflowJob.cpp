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
#include "RimWorkflowHelperProcess.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RiuMainWindow.h"
#include "RiuWorkflowJobRunner.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmPointer.h"
#include "cafPdmUiGroup.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmUiTreeOrdering.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QProcessEnvironment>
#include <QUuid>

CAF_PDM_SOURCE_INIT( RimWorkflowJob, "WorkflowJob" );

RimWorkflowJob::RimWorkflowJob()
{
    CAF_PDM_InitObject( "Job", ":/Bullet.png" );

    CAF_PDM_InitFieldNoDefault( &m_name, "Name", "Name" );

    CAF_PDM_InitFieldNoDefault( &m_taskInputs, "TaskInputs", "" );
}

RimWorkflowJob::~RimWorkflowJob()
{
    cancelJob();
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
        if ( !task || task->count() == 0 ) continue;
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

QMap<QString, QString> RimWorkflowJob::taskStates() const
{
    return m_taskStates;
}

QMap<QString, QString> RimWorkflowJob::taskErrors() const
{
    return m_taskErrors;
}

QString RimWorkflowJob::runStatus() const
{
    return m_runStatus;
}

void RimWorkflowJob::updateTaskState( const QString& runId, const QString& taskName, const QString& state, const QString& error )
{
    if ( runId != m_runId || ( state != "running" && state != "completed" && state != "failed" ) ) return;
    auto* workflow = firstAncestorOrThisOfType<RimWorkflow>();
    if ( !workflow ) return;
    bool knownTask = false;
    for ( const QJsonValue& value : workflow->graph().value( "tasks" ).toArray() )
    {
        if ( value.toObject().value( "name" ).toString() == taskName ) knownTask = true;
    }
    if ( !knownTask ) return;

    m_taskStates[taskName] = state;
    if ( state == "running" ) m_activeTask = taskName;
    if ( state == "failed" )
    {
        m_taskErrors[taskName] = error;
        m_runStatus            = "Failed: " + error;
    }
    if ( auto* window = RiuMainWindow::instance() ) window->workflowJobStateChanged( this );
}

void RimWorkflowJob::finishRun( const QString& runId, bool succeeded, bool cancelled )
{
    if ( runId != m_runId ) return;
    if ( !succeeded )
    {
        if ( m_taskStates.value( m_activeTask ) == "running" ) m_taskStates[m_activeTask] = "interrupted";
        for ( auto it = m_taskStates.begin(); it != m_taskStates.end(); ++it )
        {
            if ( it.value() == "pending" ) it.value() = "skipped";
        }
    }
    if ( cancelled )
        m_runStatus = "Cancelled";
    else if ( succeeded )
        m_runStatus = "Completed";
    else if ( !m_runStatus.startsWith( "Failed:" ) )
        m_runStatus = "Failed (see workflow log)";
    m_activeTask.clear();
    if ( auto* window = RiuMainWindow::instance() ) window->workflowJobStateChanged( this );
}

void RimWorkflowJob::cancelJob()
{
    if ( m_runner ) m_runner->cancel();
}

RimWorkflowTaskInput* RimWorkflowJob::taskInput( const QString& taskName ) const
{
    for ( RimWorkflowTaskInput* input : taskInputs() )
    {
        if ( input->taskName() == taskName ) return input;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// Make the task inputs follow the tasks of the workflow graph. Inputs of renamed tasks
/// (`renames` maps old to new names) keep their values; values of removed fields are kept aside
/// and restored if the field comes back.
//--------------------------------------------------------------------------------------------------
void RimWorkflowJob::syncTaskInputs( const QJsonArray& graphTasks, const QMap<QString, QString>& renames )
{
    // Detached values are stored as "task/field/binding"
    QMap<QString, QJsonValue> detached;
    for ( auto it = m_detachedValues.begin(); it != m_detachedValues.end(); ++it )
    {
        const QString task = it.key().section( '/', 0, 0 );
        const QString rest = it.key().section( '/', 1 );
        detached.insert( renames.value( task, task ) + "/" + rest, it.value() );
    }

    std::vector<RimWorkflowTaskInput*> existing = taskInputs();
    m_taskInputs.clearWithoutDelete();

    auto takeExisting = [&existing, &renames]( const QString& name ) -> RimWorkflowTaskInput*
    {
        for ( auto it = existing.begin(); it != existing.end(); ++it )
        {
            const QString currentName = renames.value( ( *it )->taskName(), ( *it )->taskName() );
            if ( currentName == name )
            {
                RimWorkflowTaskInput* input = *it;
                existing.erase( it );
                return input;
            }
        }
        return nullptr;
    };

    for ( const QJsonValue& value : graphTasks )
    {
        const QJsonObject task = value.toObject();
        const QString     name = task.value( "name" ).toString();

        RimWorkflowTaskInput* input = takeExisting( name );
        if ( !input ) input = new RimWorkflowTaskInput;
        input->setTaskName( name );
        input->setTaskInfo( task.value( "task_id" ).toString(), task.value( "description" ).toString() );

        QMap<QString, QJsonValue> taskDetached;
        const QString             prefix = name + "/";
        for ( auto it = detached.begin(); it != detached.end(); )
        {
            if ( it.key().startsWith( prefix ) )
            {
                taskDetached.insert( it.key().mid( prefix.size() ), it.value() );
                it = detached.erase( it );
            }
            else
            {
                ++it;
            }
        }

        input->syncFromSchema( task.value( "config_fields" ).toArray(), taskDetached );
        for ( auto it = taskDetached.begin(); it != taskDetached.end(); ++it )
            detached.insert( prefix + it.key(), it.value() );

        m_taskInputs.push_back( input );
    }

    // Inputs of removed tasks are kept aside, in case the removal is undone
    for ( RimWorkflowTaskInput* removed : existing )
    {
        const QString name = renames.value( removed->taskName(), removed->taskName() );
        for ( RimWorkflowFieldBinding* binding : removed->items() )
        {
            const QJsonValue value = binding->toJsonValue();
            if ( !value.isNull() )
                detached.insert( name + "/" + RimWorkflowTaskInput::detachedKey( binding->fieldName(), binding->classKeyword() ), value );
        }
        delete removed;
    }

    m_detachedValues = detached;
    uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// The values used to run the job, including references to ResInsight objects
//--------------------------------------------------------------------------------------------------
QJsonObject RimWorkflowJob::inputValues() const
{
    QJsonObject values;
    for ( RimWorkflowTaskInput* input : taskInputs() )
    {
        if ( input->count() > 0 ) values.insert( input->taskName(), input->jsonValues() );
    }
    return values;
}

//--------------------------------------------------------------------------------------------------
/// Values that can be saved to input.yaml
//--------------------------------------------------------------------------------------------------
QJsonObject RimWorkflowJob::literalInputValues() const
{
    QJsonObject values;
    for ( RimWorkflowTaskInput* input : taskInputs() )
    {
        if ( input->count() > 0 ) values.insert( input->taskName(), input->literalValues() );
    }
    return values;
}

//--------------------------------------------------------------------------------------------------
/// Write the input values as JSON, which is also valid YAML
//--------------------------------------------------------------------------------------------------
QString RimWorkflowJob::writeInputYaml( const QString& path ) const
{
    QFile out( path );
    if ( !out.open( QIODevice::WriteOnly | QIODevice::Truncate ) ) return {};
    out.write( QJsonDocument( inputValues() ).toJson( QJsonDocument::Indented ) );
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

    auto port = RiaApplication::instance()->activeGrpcPortNumber();
    if ( !port.has_value() )
    {
        RiaLogging::warning( "Cannot run workflow: gRPC server is not active. Enable it in Preferences." );
        return;
    }

    QString python = RimWorkflow::findPythonExecutable();
    if ( python.isEmpty() )
    {
        RiaLogging::warning( "Cannot run workflow: no Python interpreter found." );
        return;
    }

    auto runSource = workflow->prepareRunSource();
    if ( !runSource )
    {
        RiaLogging::warning( QString( "Cannot run workflow: %1" ).arg( runSource.error() ).toStdString() );
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

    m_runId = QUuid::createUuid().toString( QUuid::WithoutBraces );
    m_taskStates.clear();
    m_taskErrors.clear();
    for ( const QJsonValue& value : workflow->graph().value( "tasks" ).toArray() )
    {
        const QString taskName = value.toObject().value( "name" ).toString();
        if ( !taskName.isEmpty() ) m_taskStates.insert( taskName, "pending" );
    }
    m_activeTask.clear();
    m_runStatus = "Running";
    if ( auto* window = RiuMainWindow::instance() ) window->workflowJobStateChanged( this );

    const QStringList args = RimWorkflowHelperProcess::helperArguments(
        runSource->baseArgs + QStringList{ "--input", inputPath, "--grpc-port", QString::number( port.value() ), "--run-id", m_runId } );

    QProcessEnvironment env   = RimWorkflowHelperProcess::environment( runSource->extraPythonPath );
    const QString       label = workflow->uiName() + " / " + m_name();

    m_runner                                    = new RiuWorkflowJobRunner( label, RiuMainWindow::instance() );
    const QString                         runId = m_runId;
    const caf::PdmPointer<RimWorkflowJob> safeJob( this );
    QObject::connect( m_runner,
                      &RiuWorkflowJobRunner::taskStateChanged,
                      m_runner,
                      [safeJob, runId]( const QString& eventRunId, const QString& taskName, const QString& state, const QString& error )
                      {
                          if ( safeJob && eventRunId == runId ) safeJob->updateTaskState( runId, taskName, state, error );
                      } );
    QObject::connect( m_runner,
                      &RiuWorkflowJobRunner::runFinished,
                      m_runner,
                      [safeJob, runId]( bool succeeded, bool cancelled )
                      {
                          if ( safeJob ) safeJob->finishRun( runId, succeeded, cancelled );
                      } );
    m_runner->start( python, args, env );
}
