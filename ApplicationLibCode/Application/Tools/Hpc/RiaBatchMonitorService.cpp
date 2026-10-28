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

#include "RiaBatchMonitorService.h"

#include "Hpc/RiaBatchMonitorWorker.h"
#include "RiaLogging.h"
#include "RiaPreferencesBatch.h"
#include "RiaPreferencesOpm.h"

#include <QThread>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaBatchMonitorService::RiaBatchMonitorService()
    : m_monitorWorker( nullptr )
    , m_usageCount( 0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaBatchMonitorService::~RiaBatchMonitorService()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaBatchMonitorService::startMonitoring()
{
    m_usageCount++;
    if ( m_usageCount == 1 )
    {
        internalStart();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaBatchMonitorService::stopMonitoring()
{
    if ( m_usageCount == 1 )
    {
        internalStop();
    }
    m_usageCount--;
    if ( m_usageCount < 0 ) m_usageCount = 0;
}

//--------------------------------------------------------------------------------------------------
/// To be used when preferences are updated to activate any new settings
//--------------------------------------------------------------------------------------------------
void RiaBatchMonitorService::restartMonitoring()
{
    internalStop();
    if ( m_usageCount > 0 ) internalStart();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaBatchMonitorService::internalStop()
{
    if ( m_monitorWorker != nullptr )
    {
        m_monitorWorker->stopMonitoring();
        m_monitorWorker = nullptr;
        RiaLogging::debug( "Stopping Batch Job monitor." );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaBatchMonitorService::internalStart()
{
    auto hpcPrefs = RiaPreferencesBatch::current();

    auto schedulerType = hpcPrefs->batchScheduler();
    if ( schedulerType == RiaDefines::BatchSchedulerType::LOCAL_COMPUTER ) return;

    int interval = hpcPrefs->statusRefreshInterval();
    if ( interval < 5 ) interval = 5; // minimum interval of 5 seconds
    if ( interval > 60 ) interval = 60; // maximum interval of 60 seconds

    m_monitorWorker = new RiaBatchMonitorWorker( schedulerType, interval, RiaPreferencesOpm::current()->wslOptions() );
    auto thread     = new QThread( this );

    m_monitorWorker->moveToThread( thread );

    connect( thread, &QThread::started, m_monitorWorker, &RiaBatchMonitorWorker::start );
    connect( thread, &QThread::finished, m_monitorWorker, &QObject::deleteLater );
    connect( thread, &QThread::finished, thread, &QObject::deleteLater );
    connect( m_monitorWorker, &RiaBatchMonitorWorker::jobInfoUpdate, this, &RiaBatchMonitorService::onJobInfoUpdateFromMonitor );
    connect( m_monitorWorker, &RiaBatchMonitorWorker::finished, thread, &QThread::quit );

    thread->start();

    RiaLogging::debug( "Starting Batch Job monitor." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaBatchMonitorService::onJobInfoUpdateFromMonitor( const std::map<QString, RiaHpcTools::BatchJobInformation> information )
{
    emit jobInfoUpdate( information );
}
