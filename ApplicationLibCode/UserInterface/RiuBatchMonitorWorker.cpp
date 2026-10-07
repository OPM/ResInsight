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

#include "RiuBatchMonitorWorker.h"

#include "RiaHpcTools.h"

#include <QMutexLocker>
#include <QStringList>
#include <QThread>
#include <QWidget>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuBatchMonitorWorker::RiuBatchMonitorWorker( QObject* parent )
    : QObject( parent )
    , m_keepRunning( true )
    , m_monitoringIntervalSeconds( 5 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuBatchMonitorWorker::stopMonitoring()
{
    // signal to worker thread it is time to stop
    m_keepRunning = false;
    // wait for thread to finish, but don't wait forever
    QMutexLocker locker( &m_mutex );
    m_waitForStop.wait( &m_mutex, 8000 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuBatchMonitorWorker::gatherInformation()
{
    while ( m_keepRunning )
    {
        auto jobInfo = RiaHpcTools::listSlurmJobs();
        emit informationGathered( jobInfo );

        // poll exit flag 10 times per second to respond quickly to program exit
        int       i    = 0;
        const int maxI = m_monitoringIntervalSeconds * 10;
        while ( i < maxI && m_keepRunning )
        {
            QThread::msleep( 100 );
            i++;
        }
    }

    emit finished();

    m_mutex.lock();
    m_waitForStop.wakeAll();
    m_mutex.unlock();
}
