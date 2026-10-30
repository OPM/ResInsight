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

class RiaBatchMonitorWorker;

//==================================================================================================
/// Service that gives regular updates (interval set in preferences) of running batch scheduler jobs
/// Use a single instance to not put too much load on the scheduler.
//==================================================================================================
class RiaBatchMonitorService : public QObject, public caf::SignalEmitter
{
    Q_OBJECT

public:
    // static interface, get the main monitor instance from the main application
    static RiaBatchMonitorService* instance();
    // tell service to start polling if not running
    static void start();
    // tell service to stop polling, will stop if usage count goes to zero
    static void stop();

public:
    RiaBatchMonitorService();
    ~RiaBatchMonitorService();

    void stopMonitoring();
    void startMonitoring();

    // to be called after preferences have been changed
    void restartMonitoring();

    // to be used on application exit
    void shutDown();

    // non-QObjects: connect to this to get job list updates from batch queue monitor
    caf::Signal<const RiaHpcTools::BatchJobInformationMap&> jobInfoUpdateSignal;

signals:
    // QObjects: connect to this to get job list updates from batch queue monitor
    void jobInfoUpdate( const RiaHpcTools::BatchJobInformationMap& information );

protected:
    void internalStop();
    void internalStart();

private slots:
    void onJobInfoUpdateFromMonitor( const RiaHpcTools::BatchJobInformationMap information );

private:
    RiaBatchMonitorWorker* m_monitorWorker;
    int                    m_usageCount;
};
