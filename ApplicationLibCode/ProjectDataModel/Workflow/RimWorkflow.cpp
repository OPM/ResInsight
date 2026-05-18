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

#include "RimWorkflow.h"

#include "RimWorkflowJob.h"
#include "RimWorkflowTaskInput.h"

#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "cafCmdFeatureMenuBuilder.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QProcessEnvironment>
#include <QStandardPaths>

CAF_PDM_SOURCE_INIT( RimWorkflow, "Workflow" );

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

RimWorkflow::RimWorkflow()
{
    CAF_PDM_InitObject( "Workflow", ":/Folder.png" );

    CAF_PDM_InitFieldNoDefault( &m_name, "Name", "Name" );
    m_name.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_description, "Description", "Description" );
    m_description.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_workflowDirectory, "WorkflowDirectory", "Directory" );
    m_workflowDirectory.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_loadError, "LoadError", "Load Error" );
    m_loadError.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_jobs, "Jobs", "" );
}

QString RimWorkflow::name() const
{
    return m_name();
}

void RimWorkflow::setWorkflowDirectory( const QString& directory )
{
    m_workflowDirectory = directory;
    QString name        = QFileInfo( QDir( directory ).absolutePath() ).fileName();
    m_name              = name;
    setUiName( name );
}

QString RimWorkflow::workflowDirectory() const
{
    return m_workflowDirectory().path();
}

std::vector<RimWorkflowJob*> RimWorkflow::jobs() const
{
    std::vector<RimWorkflowJob*> result;
    result.reserve( m_jobs.size() );
    for ( RimWorkflowJob* j : m_jobs.childrenByType() )
    {
        if ( j ) result.push_back( j );
    }
    return result;
}

void RimWorkflow::addJob( RimWorkflowJob* job )
{
    if ( job ) m_jobs.push_back( job );
}

void RimWorkflow::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicNewWorkflowJobFeature";
}

bool RimWorkflow::loadFromDirectory( QString* errorMessage )
{
    m_jobs.deleteChildren();
    m_loadError = "";

    const QString dir = workflowDirectory();
    if ( dir.isEmpty() ) return false;

    QString python = findPythonExecutable();
    if ( python.isEmpty() )
    {
        m_loadError = "No usable Python interpreter found";
        RiaLogging::warning( QString( "Workflow '%1': %2" ).arg( dir, m_loadError() ).toStdString() );
        if ( errorMessage ) *errorMessage = m_loadError;
        return false;
    }

    QStringList args{ "-m", "rips.taskmaestro_helper", "introspect", dir };

    QProcess            proc;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    proc.setProcessEnvironment( env );
    proc.start( python, args );
    if ( !proc.waitForStarted( 10000 ) )
    {
        m_loadError = QString( "Could not launch '%1'" ).arg( python );
        RiaLogging::warning( QString( "Workflow '%1': %2" ).arg( dir, m_loadError() ).toStdString() );
        if ( errorMessage ) *errorMessage = m_loadError;
        return false;
    }
    if ( !proc.waitForFinished( 30000 ) )
    {
        proc.kill();
        m_loadError = "Introspect helper timed out";
        RiaLogging::warning( QString( "Workflow '%1': %2" ).arg( dir, m_loadError() ).toStdString() );
        if ( errorMessage ) *errorMessage = m_loadError;
        return false;
    }

    const QByteArray stdoutBytes = proc.readAllStandardOutput();
    if ( proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0 )
    {
        m_loadError = QString::fromUtf8( proc.readAllStandardError() ).trimmed();
        if ( m_loadError().isEmpty() ) m_loadError = "Introspect helper failed";
        RiaLogging::warning( QString( "Workflow '%1' introspect failed: %2" ).arg( dir, m_loadError() ).toStdString() );
        if ( errorMessage ) *errorMessage = m_loadError;
        return false;
    }

    QJsonParseError     parseErr{};
    const QJsonDocument doc = QJsonDocument::fromJson( stdoutBytes, &parseErr );
    if ( parseErr.error != QJsonParseError::NoError || !doc.isObject() )
    {
        m_loadError = QString( "Invalid JSON from introspect: %1" ).arg( parseErr.errorString() );
        if ( errorMessage ) *errorMessage = m_loadError;
        return false;
    }

    const QJsonObject root = doc.object();
    if ( root.contains( "name" ) ) m_name = root.value( "name" ).toString();
    if ( root.contains( "description" ) ) m_description = root.value( "description" ).toString();
    setUiName( m_name() );

    std::vector<RimWorkflowTaskInput*> taskInputs;
    for ( const QJsonValue& tv : root.value( "tasks" ).toArray() )
    {
        const QJsonObject taskObj = tv.toObject();
        auto*             input   = new RimWorkflowTaskInput;
        input->setTaskName( taskObj.value( "name" ).toString() );
        input->buildFromSchema( taskObj.value( "config_fields" ).toArray() );
        taskInputs.push_back( input );
    }

    auto* job = new RimWorkflowJob;
    job->setJobName( "Default" );
    job->setTaskInputs( taskInputs );
    m_jobs.push_back( job );

    RiaLogging::info(
        QString( "Loaded workflow '%1' (%2 tasks with config) from %3" ).arg( m_name() ).arg( taskInputs.size() ).arg( dir ).toStdString() );

    return true;
}
