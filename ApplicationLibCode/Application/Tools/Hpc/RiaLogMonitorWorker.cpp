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

#include "RiaLogMonitorWorker.h"

#include "Hpc/RiaHpcTools.h"

#include <QMutexLocker>
#include <QStringList>
#include <QThread>
#include <QWidget>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaLogMonitorWorker::RiaLogMonitorWorker( int interval, QObject* parent )
    : QObject( parent )
    , m_keepRunning( true )
    , m_monitoringIntervalSeconds( interval )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaLogMonitorWorker::stopMonitoring()
{
    QMutexLocker locker( &m_runMutex );
    // signal to worker thread it is time to stop
    m_keepRunning = false;
    // wait for thread to finish, but don't wait forever
    m_waitForStop.wait( &m_runMutex, 8000 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaLogMonitorWorker::start()
{
    const int maxI = m_monitoringIntervalSeconds * 5;

    while ( m_keepRunning )
    {
        // get a local copy of the file list
        QMutexLocker locker( &m_listMutex );
        auto         fileList = m_fileList;
        locker.unlock();

        // check if files exist and read new lines from them
        for ( auto& [fileName, streamPos] : fileList )
        {
            if ( !std::filesystem::exists( fileName ) ) continue;

            std::ifstream file( fileName, std::ios::in );
            if ( !file.is_open() ) continue;

            // Move to last read position
            file.seekg( streamPos );

            QStringList logLines;

            std::string line;
            while ( std::getline( file, line ) )
            {
                logLines.append( QString::fromStdString( line ) );
            }

            streamPos = file.tellg();
            file.close();

            if ( logLines.isEmpty() ) continue;

            emit logUpdate( QString::fromStdString( fileName ), logLines );

            if ( !m_keepRunning ) break;
        }

        // poll exit flag 5 times per second to respond quickly to program exit
        int i = 0;
        while ( i < maxI && m_keepRunning )
        {
            QThread::msleep( 200 );
            i++;
        }
    }

    emit finished();

    m_runMutex.lock();
    m_waitForStop.wakeAll();
    m_runMutex.unlock();
}
