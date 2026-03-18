/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RimProcessQueue.h"

#include "RimProcess.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProcessQueue::RimProcessQueue()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProcessQueue* RimProcessQueue::instance()
{
    static RimProcessQueue theInstance;
    return &theInstance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimProcessQueue::queueProcess( RimProcess* process )
{
    instance()->internalQueueProcess( process );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcessQueue::stopProcess( size_t processId )
{
    instance()->internalStopProcess( processId );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcessQueue::onProcessFinished( size_t processId )
{
    instance()->internalOnProcessFinished( processId );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcessQueue::launchNextProcessIfPossible()
{
    if ( m_waitingProcesses.empty() ) return;

    if ( auto nextProcess = m_waitingProcesses.front() )
    {
        if ( nextProcess->start() )
        {
            m_runningProcesses.push_back( nextProcess );
        }
        else
        {
            // TODO - fix this
        }
        m_waitingProcesses.pop_front();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcessQueue::internalQueueProcess( RimProcess* process )
{
    QMutexLocker locker( &m_mutex );

    m_waitingProcesses.push_back( process );
    launchNextProcessIfPossible();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcessQueue::internalStopProcess( size_t processId )
{
    QMutexLocker locker( &m_mutex );

    for ( auto proc : m_waitingProcesses )
    {
        if ( proc->ID() == processId )
        {
            m_waitingProcesses.remove( proc );
            // TODO - handle callback of stopped process
        }
    }

    for ( auto proc : m_runningProcesses )
    {
        if ( proc->ID() == processId )
        {
            proc->terminate();
            // m_runningProcesses.remove( proc );
        }
    }

    launchNextProcessIfPossible();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcessQueue::internalOnProcessFinished( size_t processId )
{
    QMutexLocker locker( &m_mutex );

    bool isWaitingProcess = false;

    for ( auto proc : m_waitingProcesses )
    {
        if ( proc->ID() == processId )
        {
            m_waitingProcesses.remove( proc );
            break;
        }
    }

    for ( auto proc : m_runningProcesses )
    {
        if ( proc->ID() == processId )
        {
            m_runningProcesses.remove( proc );
            break;
        }
    }

    launchNextProcessIfPossible();
}
