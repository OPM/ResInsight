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

#include "RiaHpcTools.h"

#include "RiaPreferencesOpm.h"
#include "RiaWslTools.h"

#include "RimProcess.h"

namespace
{

QStringList runUtilityCommand( QString cmdStr, QStringList arguments )
{
    QStringList cmdList;

    if ( RiaPreferencesOpm::current()->useWsl() )
    {
        cmdList.append( RiaWslTools::wslCommand() );
        cmdList.append( RiaPreferencesOpm::current()->wslOptions() );
    }

    cmdList.append( cmdStr );
    cmdList.append( arguments );

    RimProcess proc( false /*no logging*/ );

    QString cmd = cmdList.takeFirst();
    proc.setCommand( cmd );
    if ( !cmdList.isEmpty() ) proc.addParameters( cmdList );

    if ( proc.execute() )
    {
        return proc.stdOut();
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void stopLsfJob( QString jobId )
{
    runUtilityCommand( "bkill", { jobId } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void stopSlurmJob( QString jobId )
{
    runUtilityCommand( "scancel", { jobId } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString decodeSlurmJobId( QStringList stdOut )
{
    for ( const auto& line : stdOut )
    {
        if ( line.trimmed().startsWith( "Submitted batch job" ) )
        {
            auto parts = line.split( ' ', Qt::SkipEmptyParts );
            if ( parts.size() > 3 )
            {
                return parts[3].trimmed();
            }
        }
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString decodeLsfJobId( QStringList stdOut )
{
    for ( const auto& line : stdOut )
    {
        if ( line.trimmed().startsWith( "Job <" ) )
        {
            auto parts = line.split( ' ', Qt::SkipEmptyParts );
            if ( parts.size() > 1 )
            {
                auto candidate = parts[1].trimmed();
                if ( candidate.startsWith( '<' ) && candidate.endsWith( '>' ) )
                {
                    return candidate.mid( 1, candidate.length() - 2 );
                }
            }
        }
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList listSlurmJobs()
{
    QStringList arguments;
    arguments << "--format";
    arguments << "%i %j %M %T %P %B"; // id name time state partition nodes
    arguments << "--me";
    arguments << "--noheader";

    return runUtilityCommand( "squeue", arguments );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList listLsfJobs()
{
    QStringList arguments;
    arguments << "-w";

    auto output = runUtilityCommand( "bjobs", arguments );

    QStringList jobInfo;

    bool foundStart = false;

    for ( auto line : output )
    {
        auto parts = line.split( ' ', Qt::SkipEmptyParts );
        if ( parts.size() < 8 ) continue;
        if ( !foundStart && parts[0] == "JOBID" )
        {
            foundStart = true;
            continue;
        }

        if ( foundStart )
        {
            // output was jobid user stat queue from_host exec_host job_name submit_time
            // convert to jobid job_name submit_time state queue exec_host
            QString newLine;
            newLine = parts[0] + " " + parts[6] + " " + parts[7] + " " + parts[2] + " " + parts[3] + " " + parts[5];
            jobInfo << newLine;
        }
    }
    return jobInfo;
}

} // namespace

namespace RiaHpcTools
{

//--------------------------------------------------------------------------------------------------
/// Returns available cluster queues for the given scheduler type
//--------------------------------------------------------------------------------------------------
QStringList availableQueues( RiaDefines::BatchSchedulerType scheduler )
{
    if ( scheduler == RiaDefines::BatchSchedulerType::SLURM )
    {
        auto rawOutput = runUtilityCommand( "sinfo", { "-a" } );
        return decodeSlurmQueues( rawOutput );
    }
    else if ( scheduler == RiaDefines::BatchSchedulerType::LSF )
    {
        auto rawOutput = runUtilityCommand( "bqueues", { "-w" } );
        return decodeLsfQueues( rawOutput );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList listJobs( RiaDefines::BatchSchedulerType scheduler )
{
    if ( scheduler == RiaDefines::BatchSchedulerType::SLURM )
    {
        return listSlurmJobs();
    }
    else if ( scheduler == RiaDefines::BatchSchedulerType::LSF )
    {
        return listLsfJobs();
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void stopJob( RiaDefines::BatchSchedulerType scheduler, QString jobId )
{
    if ( scheduler == RiaDefines::BatchSchedulerType::SLURM )
    {
        stopSlurmJob( jobId );
    }
    else if ( scheduler == RiaDefines::BatchSchedulerType::LSF )
    {
        stopLsfJob( jobId );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString findJobId( RiaDefines::BatchSchedulerType scheduler, QStringList stdOut )
{
    if ( scheduler == RiaDefines::BatchSchedulerType::SLURM )
    {
        return decodeSlurmJobId( stdOut );
    }
    else if ( scheduler == RiaDefines::BatchSchedulerType::LSF )
    {
        return decodeLsfJobId( stdOut );
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList decodeSlurmQueues( QStringList stdOut )
{
    std::set<QString> queues;
    bool              foundStart = false;
    for ( auto& line : stdOut )
    {
        auto parts = line.split( " " );
        if ( parts.isEmpty() ) continue;

        auto firstPart = parts[0].trimmed();

        if ( foundStart )
        {
            if ( firstPart.endsWith( '*' ) )
            {
                firstPart = firstPart.removeAt( firstPart.length() - 1 );
            }
            queues.insert( firstPart );
            continue;
        }
        if ( firstPart == "PARTITION" )
        {
            foundStart = true;
            continue;
        }
    }

    QStringList retList;
    for ( auto& q : queues )
    {
        retList.append( q );
    }

    return retList;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList decodeLsfQueues( QStringList stdOut )
{
    std::set<QString> queues;
    bool              foundStart = false;
    for ( auto& line : stdOut )
    {
        auto parts = line.split( " " );
        if ( parts.isEmpty() ) continue;
        auto firstPart = parts[0].trimmed();

        if ( foundStart )
        {
            queues.insert( firstPart );
            continue;
        }
        if ( firstPart == "QUEUE_NAME" )
        {
            foundStart = true;
            continue;
        }
    }

    QStringList retList;
    for ( auto& q : queues )
    {
        retList.append( q );
    }

    return retList;
}

} // namespace RiaHpcTools
