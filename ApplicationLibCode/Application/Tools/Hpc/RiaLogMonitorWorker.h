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

#pragma once

#include "Hpc/RiaHpcTools.h"

#include <atomic>
#include <iosfwd>
#include <utility>

#include <QMutex>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QWaitCondition>

class QWidget;

class RiaLogMonitorWorker : public QObject
{
    Q_OBJECT

public:
    explicit RiaLogMonitorWorker( int interval, QObject* parent = nullptr );

    int addLogFileToWatch( const std::string& fileName );
    int removeLogFileToWatch( const std::string& fileName );

    void stopMonitoring();

public slots:
    void start();

signals:
    void logUpdate( QString fileName, const QStringList logLines );
    void finished();

private:
    std::atomic<bool> m_keepRunning;
    int               m_monitoringIntervalSeconds;
    QMutex            m_runMutex;
    QWaitCondition    m_waitForStop;

    QMutex                                              m_listMutex;
    std::vector<std::pair<std::string, std::streampos>> m_fileList;
};
