/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026    Equinor ASA
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

#include <list>

#include <QMutex>

class RimProcess;

class RimProcessQueue
{
public:
    static size_t queueProcess( RimProcess* process );
    static void   stopProcess( size_t processId );
    static void   onProcessFinished( size_t processId );

protected:
    RimProcessQueue();
    static RimProcessQueue* instance();

    size_t internalQueueProcess( RimProcess* process );
    void   internalOnProcessFinished( size_t processId );
    void   internalStopProcess( size_t processId );

private:
    void launchNextProcessIfPossible();

    std::list<RimProcess*> m_waitingProcesses;
    std::list<RimProcess*> m_runningProcesses;

    QMutex m_mutex;
};
