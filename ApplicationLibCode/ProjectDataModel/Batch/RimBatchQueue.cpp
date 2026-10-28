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

#include "RiaPreferencesBatch.h"
#include "RiaPreferencesOpm.h"
#include "RiaWslTools.h"

#include "ProcessControl/RimProcess.h"
#include "RimBatchQueueLocal.h"
#include "RimBatchQueueLsf.h"
#include "RimBatchQueueSlurm.h"

#include <QDateTime>
#include <QFile>

CAF_PDM_ABSTRACT_SOURCE_INIT( RimBatchQueue, "BatchQueue" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimBatchQueue::RimBatchQueue( RiaDefines::BatchSchedulerType schedulerType )
    : m_schedulerType( schedulerType )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimBatchQueue* RimBatchQueue::createBatchQueue()
{
    switch ( RiaPreferencesBatch::current()->batchScheduler() )
    {
        case RiaDefines::BatchSchedulerType::LSF:
            return new RimBatchQueueLsf();

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
    auto proc = std::make_unique<RimProcess>( true, monitor );

    QStringList cmdList;
    cmdList.append( command );

    QString cmd = cmdList.takeFirst();
    proc->setCommand( cmd );
    proc->setWslOptions( RiaPreferencesOpm::current()->wslOptions() );
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
std::pair<bool, QStringList> RimBatchQueue::runCommand( QStringList command )
{
    RimProcess proc( false /*no logging*/ );

    QStringList cmdList;
    cmdList.append( command );

    QString cmd = cmdList.takeFirst();
    proc.setCommand( cmd );
    proc.setWslOptions( RiaPreferencesOpm::current()->wslOptions() );
    if ( !cmdList.isEmpty() ) proc.addParameters( cmdList );

    if ( proc.execute() )
    {
        return { true, proc.stdOut() };
    }

    return { false, { QString( "Failed to start command." ), cmdList.join( " " ) } };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, QString> RimBatchQueue::buildLaunchScript( QString workDir )
{
    // write launch script to the working directory
    QString filename = workDir + "/launch_job.sh";

    QFile file( filename );
    if ( !file.open( QIODevice::WriteOnly ) )
    {
        return { false, QString( "Failed to create launch script file %1." ).arg( filename ) };
    }

    // build launch script
    QTextStream out( &file );

    out << "#!/bin/sh\n";

    auto additionalOptions = RiaPreferencesBatch::current()->batchSchedulerOptions();
    if ( !additionalOptions.isEmpty() )
    {
        out << launchScriptDirective() << " " << additionalOptions << "\n\n";
    }

    if ( m_process != nullptr )
    {
        QString cmdLine = m_process->command();

        for ( auto& p : m_process->parameters() )
        {
            cmdLine += " " + p;
        }
        out << cmdLine << "\n";
    }
    out << "\n";

    file.setPermissions( file.permissions() | QFileDevice::ExeOwner | QFileDevice::ExeUser );

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBatchQueue::readStdOutErrIntoProcessLog()
{
    if ( !m_stdOutFileName.isEmpty() )
    {
        QFile file( m_stdOutFileName );
        if ( file.open( QIODevice::ReadOnly ) )
        {
            QTextStream in( &file );
            while ( !in.atEnd() )
            {
                m_process->appendStdOut( in.readLine() );
            }
            file.close();
        }
    }

    if ( !m_stdErrFileName.isEmpty() )
    {
        QFile file( m_stdErrFileName );
        if ( file.open( QIODevice::ReadOnly ) )
        {
            QTextStream in( &file );
            while ( !in.atEnd() )
            {
                m_process->appendStdErr( in.readLine() );
            }
            file.close();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::BatchSchedulerType RimBatchQueue::schedulerType() const
{
    return m_schedulerType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBatchQueue::cleanUpOldLogFiles()
{
    if ( !m_stdOutFileName.isEmpty() && QFile::exists( m_stdOutFileName ) )
    {
        QFile::remove( m_stdOutFileName );
    }
    if ( !m_stdErrFileName.isEmpty() && QFile::exists( m_stdErrFileName ) )
    {
        QFile::remove( m_stdErrFileName );
    }
}
