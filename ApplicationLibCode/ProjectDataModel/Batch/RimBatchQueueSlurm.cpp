/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RimBatchQueueSlurm.h"

#include "Hpc/RiaBatchMonitorService.h"
#include "Hpc/RiaHpcTools.h"
#include "RiaLogging.h"
#include "RiaPreferencesBatch.h"
#include "RiaPreferencesOpm.h"
#include "RiaWslTools.h"

#include "ProcessControl/RimProcess.h"
#include "RimBatchProcessMonitor.h"

CAF_PDM_SOURCE_INIT( RimBatchQueueSlurm, "BatchQueueSlurm" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimBatchQueueSlurm::RimBatchQueueSlurm( RiaDefines::BatchSchedulerType schedulerType )
    : RimBatchQueue( schedulerType )
    , m_isBatchMonitorActivated( false )
{
    RiaBatchMonitorService::instance()->jobInfoUpdateSignal.connect( this, &RimBatchQueueSlurm::onJobInfoUpdate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimBatchQueueSlurm::~RimBatchQueueSlurm()
{
    if ( m_isBatchMonitorActivated )
    {
        m_isBatchMonitorActivated = false;
        RiaBatchMonitorService::stop();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBatchQueueSlurm::queueProcess( std::shared_ptr<RimProcess> process, int nProcessesPerTask )
{
    if ( process == nullptr ) return;

    m_process    = process;
    m_monitor    = std::make_shared<RimBatchProcessMonitor>( this );
    m_jobId      = "";
    m_submitTime = QDateTime();

    auto useWsl = RiaPreferencesOpm::current()->useWsl();
    auto prefs  = RiaPreferencesBatch::current();

    bool blockingWait = prefs->jobMonitoring() == RiaDefines::BatchJobMonitoringType::BLOCKING_WAIT;

    QString workDir = m_process->workingDirectory();

    // build launch script
    auto [builtOk, script] = buildLaunchScript( workDir );
    if ( !builtOk )
    {
        m_monitor->finished( 1, QProcess::ExitStatus::NormalExit );
        RiaLogging::warning( QString( script ).toStdString() );
        return;
    }

    QString jobName  = generateJobName();
    m_stdOutFileName = QString( "%1/%2.out" ).arg( workDir ).arg( jobName );
    m_stdErrFileName = QString( "%1/%2.err" ).arg( workDir ).arg( jobName );
    cleanUpOldLogFiles();

    QString stdOut = m_stdOutFileName;
    QString stdErr = m_stdErrFileName;

    if ( useWsl )
    {
        workDir = RiaWslTools::convertToWslPath( workDir );
        script  = RiaWslTools::convertToWslPath( script );
        stdOut  = RiaWslTools::convertToWslPath( stdOut );
        stdErr  = RiaWslTools::convertToWslPath( stdErr );
    }

    QStringList arguments =
        generateCommand( script, jobName, workDir, stdOut, stdErr, prefs->queueName(), prefs->exclusiveJob(), nProcessesPerTask, blockingWait );

    if ( blockingWait )
    {
        // start process async and wait for it to finish using a process monitor
        auto [batchProcess, output] = runCommand( arguments, m_monitor );

        if ( !batchProcess )
        {
            m_monitor->finished( 1, QProcess::ExitStatus::NormalExit );
            RiaLogging::warning( output.toStdString() );
            m_batchProcess = nullptr;
        }
        else
        {
            // notify original process that we have started
            if ( m_process->monitor() ) m_process->monitor()->started();
            m_batchProcess = std::move( batchProcess );
            m_submitTime   = QDateTime::currentDateTime();
        }
    }
    else
    {
        // run submit command and start batch monitor service to monitor job state
        auto [result, stdOut] = runCommand( arguments );
        if ( result )
        {
            // notify original process that we have started
            if ( m_process->monitor() ) m_process->monitor()->started();
            // get job id from submit command output
            m_jobId = RiaHpcTools::findJobId( schedulerType(), stdOut );
            if ( m_jobId.isEmpty() )
            {
                m_monitor->finished( 1, QProcess::NormalExit );
            }
            else
            {
                RiaBatchMonitorService::start();
                m_isBatchMonitorActivated = true;
                m_submitTime              = QDateTime::currentDateTime();
            }
        }
        else
        {
            m_monitor->finished( 1, QProcess::NormalExit );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimBatchQueueSlurm::generateCommand( QString script,
                                                 QString jobName,
                                                 QString workDir,
                                                 QString stdOutLog,
                                                 QString stdErrLog,
                                                 QString queueName,
                                                 bool    exclusive,
                                                 int     numberOfProcesses,
                                                 bool    blockingWait )
{
    QStringList arguments;
    arguments << "sbatch";
    arguments << "-p";
    arguments << queueName;
    arguments << "-J";
    arguments << jobName;

    if ( exclusive )
    {
        arguments << "--exclusive";
    }
    if ( !workDir.isEmpty() )
    {
        arguments << "-D";
        arguments << workDir;
        arguments << "-o";
        arguments << stdOutLog;
        arguments << "-e";
        arguments << stdErrLog;
    }

    arguments << "-n";
    arguments << QString( "%1" ).arg( numberOfProcesses );

    if ( blockingWait )
    {
        arguments << "--wait";
    }

    arguments << script;
    return arguments;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimBatchQueueSlurm::jobId()
{
    if ( m_batchProcess && m_jobId.isEmpty() )
    {
        m_jobId = RiaHpcTools::findJobId( schedulerType(), m_batchProcess->stdOut() );
    }
    return m_jobId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBatchQueueSlurm::stopProcess()
{
    if ( m_batchProcess && m_batchProcess->isRunning() )
    {
        auto id = jobId();

        if ( !id.isEmpty() )
        {
            RiaLogging::info( QString( "Stopping batch job %1" ).arg( id ).toStdString() );
            RiaHpcTools::stopJob( schedulerType(), id, RiaPreferencesOpm::current()->wslOptions() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBatchQueueSlurm::setFinished( bool runOk )
{
    if ( !m_process || !m_process->monitor() ) return;

    if ( m_isBatchMonitorActivated )
    {
        m_isBatchMonitorActivated = false;
        RiaBatchMonitorService::stop();
    }

    readStdOutErrIntoProcessLog();
    if ( runOk )
    {
        RiaLogging::info( QString( "Batch job %1 done." ).arg( jobId() ).toStdString() );
        m_process->monitor()->finished( 0, QProcess::ExitStatus::NormalExit );
    }
    else
    {
        if ( jobId().isEmpty() )
            RiaLogging::error( "Failed to submit batch job, check batch scheduler configuration." );
        else
            RiaLogging::warning( QString( "Batch job %1 failed." ).arg( jobId() ).toStdString() );

        m_process->monitor()->finished( 1, QProcess::ExitStatus::CrashExit );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimBatchQueueSlurm::launchScriptDirective() const
{
    return "#SBATCH";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBatchQueueSlurm::onJobInfoUpdate( const SignalEmitter* emitter, const std::map<QString, RiaHpcTools::BatchJobInformation>& information )
{
    if ( !m_isBatchMonitorActivated ) return;

    if ( !information.contains( m_jobId ) )
    {
        // should we wait for job to show up in job queue?
        if ( m_submitTime.isValid() &&
             m_submitTime.secsTo( QDateTime::currentDateTime() ) < RiaPreferencesBatch::current()->secondsToWaitForSubmit() )
        {
            return;
        }

        setFinished( true );
    }
}
