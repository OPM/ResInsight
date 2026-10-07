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

#include "RiaHpcTools.h"
#include "RiaLogging.h"
#include "RiaPreferencesHpc.h"
#include "RiaPreferencesOpm.h"
#include "RiaWslTools.h"

#include "ProcessControl/RimProcess.h"
#include "ProcessControl/RimProcessMonitor.h"
#include "RimBatchProcessMonitor.h"

CAF_PDM_SOURCE_INIT( RimBatchQueueSlurm, "BatchQueueSlurm" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimBatchQueueSlurm::RimBatchQueueSlurm()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimBatchQueueSlurm::~RimBatchQueueSlurm()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBatchQueueSlurm::queueProcess( std::shared_ptr<RimProcess> process, int numberOfProcesses )
{
    m_process = process;
    m_monitor = std::make_shared<RimBatchProcessMonitor>( this );

    auto useWsl = RiaPreferencesOpm::current()->useWsl();

    // get settings
    auto prefs = RiaPreferencesHpc::current();

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

    QString stdOut = m_stdOutFileName;
    QString stdErr = m_stdErrFileName;

    if ( useWsl )
    {
        workDir = RiaWslTools::convertToWslPath( workDir );
        script  = RiaWslTools::convertToWslPath( script );
        stdOut  = RiaWslTools::convertToWslPath( stdOut );
        stdErr  = RiaWslTools::convertToWslPath( stdErr );
    }

    QStringList arguments;
    arguments << "sbatch";
    arguments << "-p";
    arguments << prefs->queueName();
    arguments << "-J";
    arguments << jobName;

    if ( prefs->exclusiveJob() )
    {
        arguments << "--exclusive";
    }
    if ( !workDir.isEmpty() )
    {
        arguments << "-D";
        arguments << workDir;
        arguments << "-o";
        arguments << stdOut;
        arguments << "-e";
        arguments << stdErr;
    }

    arguments << "-n";
    arguments << QString( "%1" ).arg( numberOfProcesses );
    arguments << "--wait";
    arguments << script;

    auto [batchProcess, output] = runCommand( arguments, m_monitor );

    if ( !batchProcess )
    {
        m_monitor->finished( 1, QProcess::ExitStatus::NormalExit );
        RiaLogging::warning( output.toStdString() );
    }
    else
    {
        m_process->monitor()->started();
        m_batchProcess = std::move( batchProcess );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBatchQueueSlurm::stopProcess()
{
    if ( m_batchProcess && m_process && m_process->monitor() )
    {
        auto jobId = RiaHpcTools::decodeSlurmJobId( m_batchProcess->stdOut() );

        if ( !jobId.isEmpty() )
        {
            RiaLogging::info( QString( "Stopping slurm job %1" ).arg( jobId ).toStdString() );
            RiaHpcTools::stopSlurmJob( jobId );
        }

        m_process->monitor()->finished( 1, QProcess::ExitStatus::CrashExit );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBatchQueueSlurm::setFinished( bool runOk )
{
    if ( m_batchProcess && m_process && m_process->monitor() )
    {
        auto jobId = RiaHpcTools::decodeSlurmJobId( m_batchProcess->stdOut() );

        readStdOutErrIntoProcessLog();

        if ( runOk )
        {
            RiaLogging::info( QString( "Slurm job %1 completed successfully" ).arg( jobId ).toStdString() );
            m_process->monitor()->finished( 0, QProcess::ExitStatus::NormalExit );
        }
        else
        {
            RiaLogging::warning( QString( "Slurm job %1 failed." ).arg( jobId ).toStdString() );
            m_process->monitor()->finished( 1, QProcess::ExitStatus::CrashExit );
        }
    }
}
