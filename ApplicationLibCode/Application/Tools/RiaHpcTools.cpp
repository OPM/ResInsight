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

#include "RiaWslTools.h"

#include "RimProcess.h"

namespace
{

QStringList runUtilityCommand( QString cmdStr, QStringList arguments, const RiaWslTools::WslOptions& wslOptions )
{
    RimProcess proc( false /*no logging*/ );
    proc.setWslOptions( wslOptions );
    proc.setCommand( cmdStr );

    if ( !arguments.isEmpty() ) proc.addParameters( arguments );

    if ( proc.execute() )
    {
        return proc.stdOut();
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void stopLsfJob( QString jobId, const RiaWslTools::WslOptions& wslOptions )
{
    runUtilityCommand( "bkill", { jobId }, wslOptions );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void stopSlurmJob( QString jobId, const RiaWslTools::WslOptions& wslOptions )
{
    runUtilityCommand( "scancel", { jobId }, wslOptions );
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
QStringList listSlurmJobs( const RiaWslTools::WslOptions& wslOptions )
{
    QStringList arguments;
    arguments << "--format";
    arguments << "%i %j %M %T %P %B"; // id name runtime state partition nodes
    arguments << "--me";
    arguments << "--noheader";

    return runUtilityCommand( "squeue", arguments, wslOptions );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList listLsfJobs( const RiaWslTools::WslOptions& wslOptions )
{
    QStringList arguments;
    arguments << "-o";
    arguments << "jobid:10 job_name:25 run_time:12 stat:8 queue:15 exec_host:20"; // id name runtime state partition nodes
    arguments << "-noheader";

    return runUtilityCommand( "bjobs", arguments, wslOptions );
}

} // namespace

namespace RiaHpcTools
{

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
BatchJobInformation::BatchJobInformation( QStringList parts )
{
    if ( parts.size() < 6 ) return;
    id      = parts[0];
    name    = parts[1];
    runTime = parts[2];
    status  = parts[3];
    queue   = parts[4];
    where   = parts[5];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QStringList BatchJobInformation::asStringList() const
{
    QStringList retList;
    retList.append( id );
    retList.append( name );
    retList.append( runTime );
    retList.append( status );
    retList.append( queue );
    retList.append( where );

    return retList;
}

//--------------------------------------------------------------------------------------------------
/// Returns available cluster queues for the given scheduler type
//--------------------------------------------------------------------------------------------------
QStringList availableQueues( RiaDefines::BatchSchedulerType scheduler, const RiaWslTools::WslOptions& wslOptions )
{
    if ( scheduler == RiaDefines::BatchSchedulerType::SLURM )
    {
        auto rawOutput = runUtilityCommand( "sinfo", { "-a" }, wslOptions );
        return decodeSlurmQueues( rawOutput );
    }
    else if ( scheduler == RiaDefines::BatchSchedulerType::LSF )
    {
        auto rawOutput = runUtilityCommand( "bqueues", { "-w" }, wslOptions );
        return decodeLsfQueues( rawOutput );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::map<QString, BatchJobInformation> listJobs( RiaDefines::BatchSchedulerType scheduler, const RiaWslTools::WslOptions& wslOptions )
{
    if ( scheduler == RiaDefines::BatchSchedulerType::SLURM )
    {
        QStringList jobLines = listSlurmJobs( wslOptions );
        return decodeSlurmJobs( jobLines );
    }
    else if ( scheduler == RiaDefines::BatchSchedulerType::LSF )
    {
        QStringList jobLines = listLsfJobs( wslOptions );
        return decodeLsfJobs( jobLines );
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void stopJob( RiaDefines::BatchSchedulerType scheduler, QString jobId, const RiaWslTools::WslOptions& wslOptions )
{
    if ( scheduler == RiaDefines::BatchSchedulerType::SLURM )
    {
        stopSlurmJob( jobId, wslOptions );
    }
    else if ( scheduler == RiaDefines::BatchSchedulerType::LSF )
    {
        stopLsfJob( jobId, wslOptions );
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
                firstPart.chop( 1 );
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::map<QString, BatchJobInformation> decodeSlurmJobs( QStringList stdOut )
{
    std::map<QString, BatchJobInformation> jobs;
    for ( auto& line : stdOut )
    {
        std::vector<QString> statusLine;
        auto                 parts = line.split( ' ', Qt::SkipEmptyParts );
        if ( parts.size() == 0 ) continue;

        jobs[parts[0]] = BatchJobInformation( parts );
    }
    return jobs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::map<QString, BatchJobInformation> decodeLsfJobs( QStringList stdOut )
{
    std::map<QString, BatchJobInformation> jobs;

    for ( auto line : stdOut )
    {
        line = line.trimmed();
        if ( line.isEmpty() ) continue;

        QStringList parts;
        // jobid
        parts.append( line.mid( 0, 10 ).trimmed() );
        // job name
        parts.append( line.mid( 11, 25 ).trimmed() );
        // run time
        parts.append( line.mid( 37, 12 ).trimmed() );
        // status
        parts.append( line.mid( 50, 8 ).trimmed() );
        // queue
        parts.append( line.mid( 59, 15 ).trimmed() );
        // exec host
        parts.append( line.mid( 75, -1 ).trimmed() );

        jobs[parts[0]] = BatchJobInformation( parts );
    }

    return jobs;
}

} // namespace RiaHpcTools
