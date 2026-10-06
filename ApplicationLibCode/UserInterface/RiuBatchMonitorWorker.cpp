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

#include <QStringList>
#include <QThread>
#include <QWidget>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuBatchMonitorWorker::RiuBatchMonitorWorker( QObject* parent )
    : QObject( parent )
    , m_keepRunning( true )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuBatchMonitorWorker::stopMonitoring()
{
    m_keepRunning = false;
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
        QThread::sleep( 5 ); // Sleep for 5 seconds before gathering information again
    }

    emit finished();
}
