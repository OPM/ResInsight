//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include "cafDataLoadController.h"

#include "cafDataLoadTask.h"
#include "cafDataLoader.h"
#include "cafPdmObject.h"

#include <QApplication>
#include <QRunnable>
#include <QThreadPool>

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
DataLoadController::DataLoadController()
    : m_taskId( 0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
DataLoadController* DataLoadController::instance()
{
    static DataLoadController* singleton = new DataLoadController;
    return singleton;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void DataLoadController::registerDataLoader( const QString&              objectType,
                                             const QString&              dataType,
                                             std::shared_ptr<DataLoader> dataLoader )
{
    std::pair<QString, QString> key = { objectType, dataType };
    m_dataLoaders[key]              = dataLoader;

    dataLoader->taskDone.connect( this, &DataLoadController::onTaskFinished );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void DataLoadController::loadData( caf::PdmObject& object, const QString& dataType, ProgressInfo& progressInfo )
{
    std::pair<QString, QString> key = { object.classKeyword(), dataType };
    auto                        it  = m_dataLoaders.find( key );
    if ( it != m_dataLoaders.end() )
    {
        std::shared_ptr<caf::DataLoader> dataLoader = it->second;

        QMutexLocker locker( &m_mutex );
        m_pendingTasksByType[dataType]++;
        locker.unlock();

        if ( dataLoader->isRunnable() )
        {
            DataLoadTask* task = new DataLoadTask( *this, *dataLoader.get(), object, dataType, m_taskId++, progressInfo );
            task->setAutoDelete( true );
            QThreadPool::globalInstance()->start( task );
        }
        else
        {
            dataLoader->loadData( object, dataType, m_taskId++, progressInfo );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void DataLoadController::blockUntilDone( const QString& dataType )
{
    int numPending = 0;
    {
        QMutexLocker locker( &m_mutex );
        numPending = m_pendingTasksByType[dataType];
    }
    while ( numPending > 0 )
    {
        {
            QMutexLocker locker( &m_mutex );
            numPending = m_pendingTasksByType[dataType];
        }

        QApplication::processEvents();
        QThread::msleep( 100 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void DataLoadController::onTaskFinished( const caf::SignalEmitter* emitter, QString dataType, int taskId )
{
    QMutexLocker locker( &m_mutex );
    m_pendingTasksByType[dataType]--;
}
