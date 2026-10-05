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

#include <QDateTime>
#include <QFile>

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
std::pair<std::unique_ptr<RimProcess>, QString> RimBatchQueue::runCommand( QStringList command, std::shared_ptr<RimProcessMonitor> monitor )
{
    QStringList cmdList;
    if ( RiaPreferencesOpm::current()->useWsl() )
    {
        cmdList.append( RiaWslTools::wslCommand() );
        cmdList.append( RiaPreferencesOpm::current()->wslOptions() );
    }

    cmdList.append( command );

    auto proc = std::make_unique<RimProcess>( true, monitor );

    QString cmd = cmdList.takeFirst();
    proc->setCommand( cmd );
    if ( !cmdList.isEmpty() ) proc->addParameters( cmdList );

    if ( proc->start() )
    {
        return { std::move( proc ), QString() };
    }

    return { nullptr, QString( "Failed to run command." ) };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, QString> RimBatchQueue::buildLaunchScript( QString workDir )
{
    // write launch script to a unique file name in the working directory
    QString filename = workDir + "/launch_" + QString::number( QDateTime::currentMSecsSinceEpoch() ) + ".sh";

    QFile file( filename );
    if ( !file.open( QIODevice::WriteOnly ) )
    {
        return { false, QString( "Failed to create launch script file %1." ).arg( filename ) };
    }

    // build launch script
    QTextStream out( &file );

    if ( m_process != nullptr )
    {
        out << "#!/bin/sh\n";

        QString cmdLine = m_process->command();

        for ( auto& p : m_process->parameters() )
        {
            cmdLine += " " + p;
        }
        out << cmdLine << "\n\n";
    }

    file.close();

    return { true, filename };
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
