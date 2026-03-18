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

#include "RiaPreferencesOpm.h"

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
    if ( process == nullptr ) return 0;
    return instance()->internalQueueProcess( process );
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

    if ( m_runningProcesses.size() >= RiaPreferencesOpm::current()->maxParallelJobs() ) return;

    auto nextProcess = m_waitingProcesses.front();
    m_waitingProcesses.pop_front();
    if ( nextProcess->start() )
    {
        m_runningProcesses.push_back( nextProcess );
    }
    else
    {
        nextProcess->notifyErrorFinish();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimProcessQueue::internalQueueProcess( RimProcess* process )
{
    QMutexLocker locker( &m_mutex );

    m_waitingProcesses.push_back( process );
    size_t processId = process->ID();
    launchNextProcessIfPossible();
    return processId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcessQueue::internalStopProcess( size_t processId )
{
    bool        isWaitingProcess = false;
    RimProcess* theProcess       = nullptr;

    {
        QMutexLocker locker( &m_mutex );

        for ( auto proc : m_waitingProcesses )
        {
            if ( proc->ID() == processId )
            {
                m_waitingProcesses.remove( proc );
                theProcess       = proc;
                isWaitingProcess = true;
                break;
            }
        }

        if ( !isWaitingProcess )
        {
            for ( auto proc : m_runningProcesses )
            {
                if ( proc->ID() == processId )
                {
                    theProcess = proc;
                    m_runningProcesses.remove( proc );
                    break;
                }
            }
        }

        launchNextProcessIfPossible();
    }

    if ( theProcess != nullptr )
    {
        if ( isWaitingProcess )
        {
            theProcess->notifyErrorFinish();
        }
        else
        {
            theProcess->terminate();
        }
    }
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
            isWaitingProcess = true;
            break;
        }
    }

    if ( !isWaitingProcess )
    {
        for ( auto proc : m_runningProcesses )
        {
            if ( proc->ID() == processId )
            {
                m_runningProcesses.remove( proc );
                break;
            }
        }
    }

    launchNextProcessIfPossible();
}
