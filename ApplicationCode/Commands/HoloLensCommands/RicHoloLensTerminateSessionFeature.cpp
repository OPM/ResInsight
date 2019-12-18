/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RicHoloLensTerminateSessionFeature.h"

#include "RicHoloLensAutoExportToSharingServerFeature.h"
#include "RicHoloLensSessionManager.h"

#include "RiaLogging.h"

#include "cafCmdFeatureManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicHoloLensTerminateSessionFeature, "RicHoloLensTerminateSessionFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHoloLensTerminateSessionFeature::isCommandEnabled()
{
    return RicHoloLensSessionManager::instance()->session() ? true : false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensTerminateSessionFeature::onActionTriggered( bool isChecked )
{
    auto* cmdFeature = dynamic_cast<RicHoloLensAutoExportToSharingServerFeature*>(
        caf::CmdFeatureManager::instance()->getCommandFeature( "RicHoloLensAutoExportToSharingServerFeature" ) );
    if ( cmdFeature )
    {
        cmdFeature->setActive( false );
    }

    RicHoloLensSessionManager::instance()->terminateSession();

    RicHoloLensSessionManager::refreshToolbarState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensTerminateSessionFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/HoloLensDisconnect24x24.png" ) );

    actionToSetup->setText( "Disconnect from HoloLens server" );
}
