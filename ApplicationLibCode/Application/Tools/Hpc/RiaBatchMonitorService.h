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
///
//==================================================================================================
class RiaBatchMonitorService : public QObject, public caf::SignalEmitter
{
    Q_OBJECT

public:
    RiaBatchMonitorService();
    ~RiaBatchMonitorService();

    static RiaBatchMonitorService* instance();
    static void                    start();
    static void                    stop();

    void stopMonitoring();
    void startMonitoring();

    // to be called after preferences have been changed
    void restartMonitoring();

    caf::Signal<const std::map<QString, RiaHpcTools::BatchJobInformation>&> jobInfoUpdateSignal;

signals:
    // connect to this to get job list updates from batch queue monitor
    void jobInfoUpdate( const std::map<QString, RiaHpcTools::BatchJobInformation>& information );

protected:
    void internalStop();
    void internalStart();

private slots:
    void onJobInfoUpdateFromMonitor( const std::map<QString, RiaHpcTools::BatchJobInformation> information );

private:
    RiaBatchMonitorWorker* m_monitorWorker;
    int                    m_usageCount;
};
