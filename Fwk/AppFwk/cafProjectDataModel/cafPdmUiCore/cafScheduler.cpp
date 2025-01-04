/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022- Equinor ASA
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

#include "cafScheduler.h"

namespace caf
{

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Scheduler::Scheduler()
    : m_blockUpdate( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Scheduler::~Scheduler()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Scheduler::blockUpdate( bool blockUpdate )
{
    m_blockUpdate = blockUpdate;
    if ( !m_blockUpdate )
    {
        startTimer( 0 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Scheduler::slotUpdateScheduledItemsWhenReady()
{
    if ( SchedulerCallable::instance()->isScheduledTaskBlocked() )
    {
        startTimer( 100 );
        return;
    }

    if ( !m_blockUpdate ) performScheduledUpdates();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Scheduler::startTimer( int msecs )
{
    if ( !m_updateTimer )
    {
        m_updateTimer.reset( new QTimer( this ) );
        connect( m_updateTimer.data(), SIGNAL( timeout() ), this, SLOT( slotUpdateScheduledItemsWhenReady() ) );
    }

    if ( !m_updateTimer->isActive() )
    {
        m_updateTimer->setSingleShot( true );
        m_updateTimer->start( msecs );
    }
}
} // namespace caf
