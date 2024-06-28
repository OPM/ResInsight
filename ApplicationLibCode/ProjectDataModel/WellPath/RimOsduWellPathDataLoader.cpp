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
void RimOsduWellPathDataLoader::loadData( caf::PdmObject& pdmObject, const QString& dataType, int taskId, caf::ProgressInfo& progressInfo )
{
    RiaApplication*   app           = RiaApplication::instance();
    RiaOsduConnector* osduConnector = app->makeOsduConnector();

    // TODO: this is weird?
    m_dataType = dataType;

    auto oWPath = dynamic_cast<RimOsduWellPath*>( &pdmObject );
    if ( oWPath )
    {
        QString trajectoryId = oWPath->wellboreTrajectoryId();
        osduConnector->requestWellboreTrajectoryParquetDataById( trajectoryId );
        m_wellPaths[trajectoryId] = oWPath;
        m_taskIds[trajectoryId]   = taskId;
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
void RimOsduWellPathDataLoader::parquetDownloadComplete( const QByteArray& contents, const QString& url, const QString& id )
{
    RiaLogging::info( QString( "Parquet download complete. Id: %1 Size: %2" ).arg( id ).arg( contents.size() ) );

    QMutexLocker lock( &m_mutex );
    int          taskId = m_taskIds[id];

    if ( !contents.isEmpty() )
    {
        m_parquetData[id] = contents;

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
