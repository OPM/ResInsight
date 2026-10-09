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
QStringList RimBatchQueueLsf::generateCommand( QString script,
                                               QString jobName,
                                               QString workDir,
                                               QString stdOutLog,
                                               QString stdErrLog,
                                               QString queueName,
                                               bool    exclusive,
                                               int     numberOfProcesses )
{
    QStringList arguments;
    arguments << "bsub";
    // the queue to use
    arguments << "-q";
    arguments << queueName;
    // the name of the job
    arguments << "-J";
    arguments << jobName;

    // should we request exclusive access to a node?
    if ( exclusive )
    {
        arguments << "-x";
    }

    // working directory and log file output
    if ( !workDir.isEmpty() )
    {
        arguments << "-cwd";
        arguments << workDir;
        arguments << "-o";
        arguments << stdOutLog;
        arguments << "-e";
        arguments << stdErrLog;
    }

    // number of tasks we are going to run (i.e. mpi processes)
    arguments << "-n";
    arguments << QString( "%1" ).arg( numberOfProcesses );

    // blocking wait
    arguments << "-K";

    // the actual script to run
    arguments << script;

    return arguments;
}
