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

#include "RicHoloLensAutoExportToSharingServerFeature.h"
#include "RicHoloLensSession.h"
#include "RicHoloLensSessionManager.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimGridView.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicHoloLensAutoExportToSharingServerFeature, "RicHoloLensAutoExportToSharingServerFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicHoloLensAutoExportToSharingServerFeature::RicHoloLensAutoExportToSharingServerFeature()
    : m_isActive( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensAutoExportToSharingServerFeature::setActive( bool enable )
{
    m_isActive = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHoloLensAutoExportToSharingServerFeature::isActive() const
{
    if ( isSessionValid() )
    {
        return m_isActive;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensAutoExportToSharingServerFeature::triggerUpdateSession()
{
    if ( m_isActive && isSessionValid() )
    {
        RimGridView* activeView = RiaApplication::instance()->activeGridView();
        if ( !activeView )
        {
            RiaLogging::error( "No active view" );
            return;
        }

        RicHoloLensSession* session = RicHoloLensSessionManager::instance()->session();

        session->updateSessionDataFromView( *activeView );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHoloLensAutoExportToSharingServerFeature::isCommandEnabled()
{
    return isSessionValid();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensAutoExportToSharingServerFeature::onActionTriggered( bool isChecked )
{
    m_isActive = !m_isActive;

    if ( !isSessionValid() )
    {
        RiaLogging::error( "No valid HoloLens session present" );
        m_isActive = false;
    }

    RimGridView* activeView = RiaApplication::instance()->activeGridView();
    if ( !activeView )
    {
        RiaLogging::error( "No active view" );
        m_isActive = false;
    }

    if ( m_isActive )
    {
        triggerUpdateSession();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensAutoExportToSharingServerFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/HoloLensSendContinously24x24.png" ) );

    actionToSetup->setText( "Export to HoloLens Server Continuously" );
    actionToSetup->setCheckable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHoloLensAutoExportToSharingServerFeature::isCommandChecked()
{
    return isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHoloLensAutoExportToSharingServerFeature::isSessionValid() const
{
    RicHoloLensSession* session = RicHoloLensSessionManager::instance()->session();
    if ( session && session->isSessionValid() )
    {
        return true;
    }
    else
    {
        return false;
    }
}
