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
//  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
//  at <http://www.gnu.org/licenses/gpl.html> for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RiuWorkflowValidationRunner.h"

#include "Workflow/RimWorkflow.h"
#include "Workflow/RimWorkflowHelperProcess.h"
#include "Workflow/RimWorkflowValidationTools.h"

#include "RiaLogging.h"

#include <QProcess>
#include <QTimer>

namespace
{
RimWorkflowIssue issueWithMessage( const QString& message )
{
    RimWorkflowIssue issue;
    issue.message = message;
    return issue;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWorkflowValidationRunner::RiuWorkflowValidationRunner( QObject* parent )
    : QObject( parent )
    , m_debounceTimer( new QTimer( this ) )
    , m_watchdogTimer( new QTimer( this ) )
{
    m_debounceTimer->setSingleShot( true );
    m_debounceTimer->setInterval( debounceMs );
    connect( m_debounceTimer, &QTimer::timeout, this, &RiuWorkflowValidationRunner::start );

    m_watchdogTimer->setSingleShot( true );
    m_watchdogTimer->setInterval( watchdogMs );
    connect( m_watchdogTimer, &QTimer::timeout, this, &RiuWorkflowValidationRunner::onTimeout );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWorkflowValidationRunner::~RiuWorkflowValidationRunner()
{
    stopProcess();
}

//--------------------------------------------------------------------------------------------------
/// Validate the current revision of the workflow after a short pause. A run in progress for an
/// older revision is stopped.
//--------------------------------------------------------------------------------------------------
void RiuWorkflowValidationRunner::schedule( RimWorkflow* workflow )
{
    if ( !workflow || !workflow->isEditable() || workflow->isValidationCurrent() )
    {
        if ( workflow == m_workflow.p() || !workflow ) m_debounceTimer->stop();
        return;
    }

    m_workflow = workflow;
    if ( m_process && ( m_runningWorkflow.p() != workflow || m_runningRevision != workflow->revision() ) ) stopProcess();
    m_debounceTimer->start();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowValidationRunner::cancel()
{
    m_debounceTimer->stop();
    stopProcess();
    m_workflow = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuWorkflowValidationRunner::isRunning() const
{
    return m_process != nullptr || m_debounceTimer->isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowValidationRunner::start()
{
    RimWorkflow* workflow = m_workflow.p();
    if ( !workflow || !workflow->isEditable() || workflow->isValidationCurrent() ) return;
    if ( m_process && m_runningWorkflow.p() == workflow && m_runningRevision == workflow->revision() ) return;
    stopProcess();

    const QString python        = RimWorkflow::findPythonExecutable();
    auto          editDirectory = workflow->editDirectory();
    if ( python.isEmpty() || !editDirectory )
    {
        const QString message = python.isEmpty() ? QString( "No usable Python interpreter found" ) : editDirectory.error();
        workflow->setValidationResult( workflow->revision(), { issueWithMessage( message ) }, false );
        emit validationFinished( workflow );
        return;
    }

    m_runningWorkflow = workflow;
    m_runningRevision = workflow->revision();

    m_process = new QProcess( this );
    m_process->setProcessEnvironment( RimWorkflowHelperProcess::environment( workflow->sourceDirectory() ) );
    connect( m_process, &QProcess::finished, this, &RiuWorkflowValidationRunner::onFinished );
    connect( m_process,
             &QProcess::errorOccurred,
             this,
             [this]( QProcess::ProcessError error )
             {
                 if ( error == QProcess::FailedToStart ) onFinished();
             } );

    m_process->start( python, RimWorkflowHelperProcess::helperArguments( { "save", "--out-dir", editDirectory.value(), "--describe" } ) );
    m_process->write( workflow->definitionJsonForSave() );
    m_process->closeWriteChannel();
    m_watchdogTimer->start();
    emit validationStarted();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowValidationRunner::onFinished()
{
    if ( !m_process ) return;
    m_watchdogTimer->stop();

    QProcess* process  = m_process;
    const int exitCode = process->exitStatus() == QProcess::NormalExit && process->error() != QProcess::FailedToStart ? process->exitCode()
                                                                                                                      : -1;
    const auto output = RimWorkflowHelperProcess::parseOutput( exitCode, process->readAllStandardOutput(), process->readAllStandardError() );
    m_process = nullptr;
    process->deleteLater();

    RimWorkflow* workflow = m_runningWorkflow.p();
    const int    revision = m_runningRevision;
    m_runningWorkflow     = nullptr;
    m_runningRevision     = -1;
    if ( !workflow || workflow->revision() != revision ) return;

    if ( output )
    {
        const auto validation = RimWorkflowValidationTools::issuesFromHelperResult( output.value(), workflow->definition().nodeNames() );
        workflow->setValidationResult( revision, validation.issues, validation.describeSucceeded );
        if ( output->value( "status" ).toString() != "invalid" ) workflow->setEditDirectoryRevision( revision );
    }
    else
    {
        RiaLogging::debug( QString( "Workflow validation failed: %1" ).arg( output.error() ).toStdString() );
        workflow->setValidationResult( revision, { issueWithMessage( output.error() ) }, false );
    }
    emit validationFinished( workflow );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowValidationRunner::onTimeout()
{
    RimWorkflow* workflow = m_runningWorkflow.p();
    const int    revision = m_runningRevision;
    stopProcess();
    if ( workflow && workflow->revision() == revision )
    {
        workflow->setValidationResult( revision, { issueWithMessage( "Validation timed out" ) }, false );
        emit validationFinished( workflow );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowValidationRunner::stopProcess()
{
    m_watchdogTimer->stop();
    if ( m_process )
    {
        QProcess* process = m_process;
        m_process         = nullptr;
        process->disconnect( this );
        process->kill();
        process->waitForFinished( 1000 );
        process->deleteLater();
    }
    m_runningWorkflow = nullptr;
    m_runningRevision = -1;
}
