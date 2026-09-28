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

#include "ProcessControl/RimProcess.h"
#include "ProcessControl/RimProcessMonitor.h"

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

    // get settings
    auto prefs = RiaPreferencesHpc::current();

    QString jobName = generateJobName();

    // build launch script
    QStringList stdIn = buildLaunchScript();

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

    arguments << "-n";
    arguments << QString( "%1" ).arg( numberOfProcesses );

    auto [result, output] = runCommand( arguments );

    if ( result )
    {
        m_process->monitor()->finished( 0, QProcess::ExitStatus::NormalExit );
    }
    else
    {
        m_process->monitor()->finished( 1, QProcess::ExitStatus::NormalExit );
    }

    RiaLogging::info( QString( output.join( "\n" ) ).toStdString() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBatchQueueSlurm::stopProcess()
{
    // RiaHpcTools::stopSlurmJob( processId );
}
