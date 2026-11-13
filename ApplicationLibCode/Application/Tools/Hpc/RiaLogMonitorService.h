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

#include "cafSignal.h"

#include <QObject>
#include <QStringList>

class RiaLogMonitorWorker;

//==================================================================================================
/// Service that gives regular updates (interval set in preferences) of the registered log files
//==================================================================================================
class RiaLogMonitorService : public QObject, public caf::SignalEmitter
{
    Q_OBJECT

public:
    // static interface, get the main monitor instance from the main application
    static RiaLogMonitorService* instance();
    // tell service to start monitoring the specified file, will start if not running
    static void start( std::string filename );
    // tell service to stop monitoring the specified file, will stop if usage count goes to zero
    static void stop( std::string filename );

public:
    RiaLogMonitorService();
    ~RiaLogMonitorService() override;

    void stopMonitoring( std::string filename );
    void startMonitoring( std::string filename );

    // to be called after preferences have been changed
    void restartMonitoring();

    // to be used on application exit
    void shutDown();

    // non-QObjects: connect to this to get log updates from the monitor
    caf::Signal<std::pair<const QString, const QStringList&>> logUpdateSignal;

signals:
    // QObjects: connect to this to get log updates from the monitor
    void logUpdate( const QString& filename, const QStringList& information );

protected:
    void internalStop();
    void internalStart();

private slots:
    void onLogUpdateFromMonitor( const QString filename, const QStringList newLines );

private:
    RiaLogMonitorWorker* m_monitorWorker;
    int                  m_usageCount;
};
