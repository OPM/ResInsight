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

#include "RiaLogMonitorService.h"

#include "Hpc/RiaLogMonitorWorker.h"
#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaPreferencesBatch.h"
#include "RiaPreferencesOpm.h"

#include <QThread>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaLogMonitorService::RiaLogMonitorService()
    : m_monitorWorker( nullptr )
    , m_usageCount( 0 )
    , logUpdateSignal( this )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaLogMonitorService::~RiaLogMonitorService()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaLogMonitorService* RiaLogMonitorService::instance()
{
    return RiaApplication::instance()->logMonitorService();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaLogMonitorService::start( std::string filename )
{
    instance()->startMonitoring( filename );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaLogMonitorService::stop( std::string filename )
{
    instance()->stopMonitoring( filename );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaLogMonitorService::startMonitoring( std::string filename )
{
    if ( m_usageCount == 0 )
    {
        internalStart();
    }
    m_usageCount = m_monitorWorker->addLogFileToWatch( filename );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaLogMonitorService::stopMonitoring( std::string filename )
{
    if ( m_monitorWorker != nullptr )
    {
        m_usageCount = m_monitorWorker->removeLogFileToWatch( filename );
    }
    if ( m_usageCount == 0 )
    {
        internalStop();
    }
}

//--------------------------------------------------------------------------------------------------
/// To be used when preferences are updated to activate any new settings
//--------------------------------------------------------------------------------------------------
void RiaLogMonitorService::restartMonitoring()
{
    internalStop();
    if ( m_usageCount > 0 ) internalStart();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaLogMonitorService::shutDown()
{
    internalStop();
    m_usageCount = 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaLogMonitorService::internalStop()
{
    if ( m_monitorWorker != nullptr )
    {
        m_monitorWorker->stopMonitoring();
        m_monitorWorker = nullptr;
        RiaLogging::debug( "Stopping Log Monitor" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaLogMonitorService::internalStart()
{
    auto hpcPrefs = RiaPreferencesBatch::current();

    auto schedulerType = hpcPrefs->batchScheduler();
    if ( schedulerType == RiaDefines::BatchSchedulerType::LOCAL_COMPUTER ) return;

    if ( !hpcPrefs->logMonitoringEnabled() ) return;

    int interval = hpcPrefs->logRefreshInterval();
    if ( interval < 5 ) interval = 5; // minimum interval of 5 seconds

    m_monitorWorker = new RiaLogMonitorWorker( interval );
    auto thread     = new QThread( this );

    m_monitorWorker->moveToThread( thread );

    connect( thread, &QThread::started, m_monitorWorker, &RiaLogMonitorWorker::start );
    connect( thread, &QThread::finished, m_monitorWorker, &QObject::deleteLater );
    connect( thread, &QThread::finished, thread, &QObject::deleteLater );
    connect( m_monitorWorker, &RiaLogMonitorWorker::logUpdate, this, &RiaLogMonitorService::onLogUpdateFromMonitor );
    connect( m_monitorWorker, &RiaLogMonitorWorker::finished, thread, &QThread::quit );

    thread->start();

    RiaLogging::debug( "Starting Log Monitor" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaLogMonitorService::onLogUpdateFromMonitor( const QString filename, const QStringList loglines )
{
    emit logUpdate( filename, loglines );
    logUpdateSignal.send( std::make_pair( filename, loglines ) );
}
