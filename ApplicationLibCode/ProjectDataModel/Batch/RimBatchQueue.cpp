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

#include "RimBatchQueue.h"

#include "RiaPreferencesHpc.h"
#include "RiaPreferencesOpm.h"
#include "RiaWslTools.h"

#include "ProcessControl/RimProcess.h"
#include "RimBatchQueueLocal.h"
#include "RimBatchQueueSlurm.h"

CAF_PDM_ABSTRACT_SOURCE_INIT( RimBatchQueue, "BatchQueue" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimBatchQueue* RimBatchQueue::createBatchQueue()
{
    switch ( RiaPreferencesHpc::current()->batchScheduler() )
    {
        case RiaDefines::BatchSchedulerType::LSF:
            break;

        case RiaDefines::BatchSchedulerType::SLURM:
            return new RimBatchQueueSlurm();

        default:
        case RiaDefines::BatchSchedulerType::LOCAL_COMPUTER:
            break;
    }
    return new RimBatchQueueLocal();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, QStringList> RimBatchQueue::runCommand( QStringList command )
{
    QStringList cmdList;
    if ( RiaPreferencesOpm::current()->useWsl() )
    {
        cmdList.append( RiaWslTools::wslCommand() );
        cmdList.append( RiaPreferencesOpm::current()->wslOptions() );
    }

    cmdList.append( command );

    RimProcess proc;

    QString cmd = cmdList.takeFirst();
    proc.setCommand( cmd );
    if ( !cmdList.isEmpty() ) proc.addParameters( cmdList );

    if ( proc.execute() )
    {
        return { true, proc.stdOut() };
    }

    return { false, { QString( "Failed to run command." ) } };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimBatchQueue::buildLaunchScript()
{
    // build launch script
    QStringList stdIn;

    if ( m_process != nullptr )
    {
        stdIn << "#!/bin/sh\n";
        stdIn << m_process->command() << "\n";
        for ( auto& p : m_process->parameters() )
        {
            stdIn << p << "\n";
        }
    }

    return stdIn;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimBatchQueue::generateJobName()
{
    if ( m_process == nullptr ) return "ResInsight_Job";

    QString candidate = m_process->description();
    candidate.replace( " ", "" );
    candidate = candidate.left( 10 );
    candidate = "RI_" + candidate;

    return candidate;
}
