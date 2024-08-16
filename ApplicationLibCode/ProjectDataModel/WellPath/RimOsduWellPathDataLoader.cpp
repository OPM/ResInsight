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

#include "RimOsduWellPathDataLoader.h"

#include "Cloud/RiaOsduConnector.h"
#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimOsduWellPath.h"

#include "RifOsduWellPathReader.h"

#include "RigWellPath.h"

#include "cafProgressInfo.h"

#include <QApplication>
#include <QObject>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimOsduWellPathDataLoader::RimOsduWellPathDataLoader()
    : caf::DataLoader()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOsduWellPathDataLoader::loadData( caf::PdmObject& pdmObject, const QString& dataType, int taskId, caf::ProgressInfo& progressInfo )
{
    RiaApplication*   app           = RiaApplication::instance();
    RiaOsduConnector* osduConnector = app->makeOsduConnector();
    if ( !osduConnector )
    {
        RiaLogging::error( "Failed to create OSDU connector" );
        return;
    }

    // TODO: this is weird?
    m_dataType = dataType;

    auto oWPath = dynamic_cast<RimOsduWellPath*>( &pdmObject );
    if ( oWPath )
    {
        connect( osduConnector,
                 SIGNAL( parquetDownloadFinished( const QByteArray&, const QString&, const QString& ) ),
                 this,
                 SLOT( parquetDownloadComplete( const QByteArray&, const QString&, const QString& ) ),
                 Qt::ConnectionType( Qt::QueuedConnection | Qt::UniqueConnection ) );

        QString trajectoryId      = oWPath->wellboreTrajectoryId();
        m_wellPaths[trajectoryId] = oWPath;
        m_taskIds[trajectoryId]   = taskId;
        osduConnector->requestWellboreTrajectoryParquetDataById( trajectoryId );
    }
    QApplication::processEvents();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimOsduWellPathDataLoader::isRunnable() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOsduWellPathDataLoader::cancel()
{
    RiaApplication*   app           = RiaApplication::instance();
    RiaOsduConnector* osduConnector = app->makeOsduConnector();

    for ( auto& [trajectoryId, taskId] : m_taskIds )
    {
        osduConnector->cancelRequestForId( trajectoryId );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOsduWellPathDataLoader::parquetDownloadComplete( const QByteArray& contents, const QString& url, const QString& id )
{
    QMutexLocker lock( &m_mutex );

    if ( m_wellPaths.find( id ) != m_wellPaths.end() && m_taskIds.find( id ) != m_taskIds.end() )
    {
        RiaLogging::info( QString( "Parquet download complete. Id: %1 Size: %2" ).arg( id ).arg( contents.size() ) );
        int taskId = m_taskIds[id];

        if ( !contents.isEmpty() )
        {
            auto oWPath                           = m_wellPaths[id];
            auto [wellPathGeometry, errorMessage] = RifOsduWellPathReader::readWellPathData( contents, oWPath->datumElevationFromOsdu() );
            if ( wellPathGeometry.notNull() )
            {
                oWPath->setWellPathGeometry( wellPathGeometry.p() );
            }
            else
            {
                RiaLogging::warning( errorMessage );
            }
        }

        taskDone.send( m_dataType, taskId );
    }
}
