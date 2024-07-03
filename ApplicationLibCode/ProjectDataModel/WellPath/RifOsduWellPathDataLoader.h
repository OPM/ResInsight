/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#pragma once

#include "RiaApplication.h"

#include "OsduImportCommands/RiaOsduConnector.h"
#include "RiaLogging.h"
#include "RimOsduWellPath.h"

#include "RifOsduWellPathReader.h"

#include "RigWellPath.h"

#include "cafDataLoader.h"
#include "cafProgressInfo.h"

#include <QApplication>
#include <QObject>
#include <qnamespace.h>

//==================================================================================================
///
///
//==================================================================================================
class RifOsduWellPathDataLoader : public QObject, public caf::DataLoader
{
    Q_OBJECT
public:
    RifOsduWellPathDataLoader()
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

    void loadData( caf::PdmObject& pdmObject, const QString& dataType, int taskId, caf::ProgressInfo& progressInfo ) override
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

    bool isRunnable() const override { return false; }

private slots:

    void parquetDownloadComplete( const QByteArray& contents, const QString& url, const QString& id )
    {
        printf( "PARQUT COMPELTE. ID: %s %d\n", id.toStdString().c_str(), contents.size() );

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

private:
    std::map<QString, QByteArray>       m_parquetData;
    std::map<QString, RimOsduWellPath*> m_wellPaths;
    std::map<QString, int>              m_taskIds;
    QString                             m_dataType;
    QMutex                              m_mutex;
};
