/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RimOsduWellLogDataLoader.h"

#include "Cloud/RiaOsduConnector.h"
#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimOsduWellLog.h"

#include "RifOsduWellLogReader.h"

#include "RigWellLogData.h"

#include "cafProgressInfo.h"

#include <QApplication>
#include <QObject>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimOsduWellLogDataLoader::RimOsduWellLogDataLoader()
    : caf::DataLoader()
{
    RiaApplication*   app           = RiaApplication::instance();
    RiaOsduConnector* osduConnector = app->makeOsduConnector();

    connect( osduConnector,
             SIGNAL( parquetDownloadFinished( const QByteArray&, const QString&, const QString& ) ),
             this,
             SLOT( parquetDownloadComplete( const QByteArray&, const QString&, const QString& ) ),
             Qt::QueuedConnection );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOsduWellLogDataLoader::loadData( caf::PdmObject& pdmObject, const QString& dataType, int taskId, caf::ProgressInfo& progressInfo )
{
    RiaApplication*   app           = RiaApplication::instance();
    RiaOsduConnector* osduConnector = app->makeOsduConnector();

    // TODO: this is weird?
    m_dataType = dataType;

    if ( RimOsduWellLog* osduWellLog = dynamic_cast<RimOsduWellLog*>( &pdmObject ) )
    {
        QString wellLogId = osduWellLog->wellLogId();
        osduConnector->requestWellLogParquetDataById( wellLogId );
        m_wellLogs[wellLogId] = osduWellLog;
        m_taskIds[wellLogId]  = taskId;
    }
    QApplication::processEvents();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimOsduWellLogDataLoader::isRunnable() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOsduWellLogDataLoader::parquetDownloadComplete( const QByteArray& contents, const QString& url, const QString& id )
{
    QMutexLocker lock( &m_mutex );

    if ( m_wellLogs.find( id ) != m_wellLogs.end() )
    {
        int taskId = m_taskIds[id];

        RiaLogging::info( QString( "Parquet download complete. Id: %1 Size: %2" ).arg( id ).arg( contents.size() ) );

        if ( !contents.isEmpty() )
        {
            auto osduWellLog                 = m_wellLogs[id];
            auto [wellLogData, errorMessage] = RifOsduWellLogReader::readWellLogData( contents );

            if ( wellLogData.notNull() )
            {
                osduWellLog->setWellLogData( wellLogData.p() );
            }
            else
            {
                RiaLogging::warning( errorMessage );
            }
        }

        taskDone.send( m_dataType, taskId );
    }
}
