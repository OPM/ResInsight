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

} // namespace RiaHpcTools
