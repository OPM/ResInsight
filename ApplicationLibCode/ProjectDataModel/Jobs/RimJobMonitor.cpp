/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025    Equinor ASA
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

#include "RimJobMonitor.h"

#include "RimGenericJob.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimJobMonitor::RimJobMonitor( RimGenericJob* job )
    : RimProcessMonitor( 0, true )
    , m_job( job )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimJobMonitor::~RimJobMonitor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimJobMonitor::readyReadStandardOutput()
{
    if ( QProcess* p = qobject_cast<QProcess*>( sender() ) )
    {
        p->setReadChannel( QProcess::StandardOutput );
        while ( p->canReadLine() )
        {
            QString line = p->readLine();
            line         = line.trimmed();
            if ( line.size() == 0 ) continue;

            m_job->processLogOutput( line );
            m_stdOut.append( line );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimJobMonitor::finished( int exitCode, QProcess::ExitStatus exitStatus )
{
    if ( m_job.notNull() )
    {
        m_job->setFinished( exitStatus == QProcess::NormalExit && exitCode == 0 );
    }

    RimProcessMonitor::finished( exitCode, exitStatus );
}
