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

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "RiuMainWindow.h"
#include "RiuWorkflowRunDialog.h"

#include "cafPdmUiPushButtonEditor.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QUuid>
#include <QWidget>

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

    CAF_PDM_InitField( &m_runButton, "RunButton", false, "Run" );
    m_runButton.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    m_runButton.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_taskInputs, "TaskInputs", "" );
}

void RimWorkflow::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_runButton )
    {
        m_runButton = false;
        runWorkflow();
    }
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

std::vector<RimWorkflowTaskInput*> RimWorkflow::taskInputs() const
{
    std::vector<RimWorkflowTaskInput*> result;
    result.reserve( m_taskInputs.size() );
    for ( RimWorkflowTaskInput* t : m_taskInputs.childrenByType() )
    {
        if ( t ) result.push_back( t );
    }
    return result;
}

bool RimWorkflow::loadFromDirectory( QString* errorMessage )
{
    m_taskInputs.deleteChildren();
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

    QProcess         proc;
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

    QJsonParseError parseErr{};
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

    for ( const QJsonValue& tv : root.value( "tasks" ).toArray() )
    {
        const QJsonObject taskObj = tv.toObject();
        auto*             input   = new RimWorkflowTaskInput;
        input->setTaskName( taskObj.value( "name" ).toString() );
        input->buildFromSchema( taskObj.value( "config_fields" ).toArray() );
        m_taskInputs.push_back( input );
    }

    RiaLogging::info( QString( "Loaded workflow '%1' (%2 tasks with config) from %3" )
                          .arg( m_name() )
                          .arg( m_taskInputs.size() )
                          .arg( dir )
                          .toStdString() );

    return true;
}

QString RimWorkflow::writeInputYaml( const QString& path ) const
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

void RimWorkflow::runWorkflow()
{
    const QString dir = workflowDirectory();
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

    QDir tmp( QDir::tempPath() );
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

    QStringList args{ "-m",
                      "rips.taskmaestro_helper",
                      "run",
                      dir,
                      "--input",
                      inputPath,
                      "--grpc-port",
                      QString::number( port.value() ) };

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    auto* dialog = new RiuWorkflowRunDialog( m_name(), RiuMainWindow::instance() );
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    dialog->show();
    dialog->start( python, args, env );
}
