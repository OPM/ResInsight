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

#include "RimBatchQueueLsf.h"

#include "RiaHpcTools.h"
#include "RiaLogging.h"
#include "RiaPreferencesHpc.h"
#include "RiaPreferencesOpm.h"
#include "RiaWslTools.h"

#include "ProcessControl/RimProcess.h"
#include "ProcessControl/RimProcessMonitor.h"
#include "RimBatchProcessMonitor.h"

CAF_PDM_SOURCE_INIT( RimBatchQueueLsf, "BatchQueueLsf" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimBatchQueueLsf::RimBatchQueueLsf()
    : RimBatchQueueSlurm( RiaDefines::BatchSchedulerType::LSF )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimBatchQueueLsf::~RimBatchQueueLsf()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBatchQueueLsf::queueProcess( std::shared_ptr<RimProcess> process, int numberOfProcesses )
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
    arguments << "bsub";
    // the queue to use
    arguments << "-q";
    arguments << prefs->queueName();
    // the name of the job
    arguments << "-J";
    arguments << jobName;

    // should we request exclusive access to a node?
    if ( prefs->exclusiveJob() )
    {
        arguments << "-x";
    }

    // working directory and log file output
    if ( !workDir.isEmpty() )
    {
        arguments << "-cwd";
        arguments << workDir;
        arguments << "-o";
        arguments << stdOut;
        arguments << "-e";
        arguments << stdErr;
    }

    // number of tasks we are going to run (i.e. mpi processes)
    arguments << "-n";
    arguments << QString( "%1" ).arg( numberOfProcesses );

    // blocking wait
    arguments << "-K";

    // the actual script to run
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
void RimBatchQueueLsf::stopProcess()
{
}
