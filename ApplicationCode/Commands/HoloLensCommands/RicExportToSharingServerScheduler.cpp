/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RicExportToSharingServerScheduler.h"

#include "RicHoloLensAutoExportToSharingServerFeature.h"
#include "RicHoloLensSession.h"
#include "RicHoloLensSessionManager.h"

#include "cafCmdFeatureManager.h"
#include "cafProgressState.h"

#include <QCoreApplication>
#include <QTimer>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicExportToSharingServerScheduler* RicExportToSharingServerScheduler::instance()
{
    static RicExportToSharingServerScheduler theInstance;

    return &theInstance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportToSharingServerScheduler::scheduleUpdateSession()
{
    RicHoloLensSession* session = RicHoloLensSessionManager::instance()->session();
    if ( session && session->isSessionValid() )
    {
        startTimer( 0 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportToSharingServerScheduler::startTimer( int msecs )
{
    if ( !m_timer )
    {
        m_timer = new QTimer( this );
        connect( m_timer, SIGNAL( timeout() ), this, SLOT( slotTriggerUpdateSessionWhenReady() ) );
    }

    if ( !m_timer->isActive() )
    {
        m_timer->setSingleShot( true );
        m_timer->start( msecs );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportToSharingServerScheduler::triggerUpdateSession()
{
    auto* cmdFeature = dynamic_cast<RicHoloLensAutoExportToSharingServerFeature*>(
        caf::CmdFeatureManager::instance()->getCommandFeature( "RicHoloLensAutoExportToSharingServerFeature" ) );
    if ( cmdFeature )
    {
        cmdFeature->triggerUpdateSession();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportToSharingServerScheduler::slotTriggerUpdateSessionWhenReady()
{
    if ( caf::ProgressState::isActive() )
    {
        startTimer( 100 );
        return;
    }

    triggerUpdateSession();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicExportToSharingServerScheduler::~RicExportToSharingServerScheduler()
{
    delete m_timer;
}
