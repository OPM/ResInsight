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

#include "RicHoloLensCreateDummyFileBackedSessionFeature.h"

#include "RicHoloLensSessionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicHoloLensCreateDummyFiledBackedSessionFeature, "RicHoloLensCreateDummyFiledBackedSessionFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHoloLensCreateDummyFiledBackedSessionFeature::isCommandEnabled()
{
    return RicHoloLensSessionManager::instance()->session() ? false : true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensCreateDummyFiledBackedSessionFeature::onActionTriggered( bool isChecked )
{
    RicHoloLensSessionManager::instance()->createDummyFileBackedSession();

    RicHoloLensSessionManager::refreshToolbarState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensCreateDummyFiledBackedSessionFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/HoloLensConnect24x24.png" ) );

    actionToSetup->setText( "Create File-Backed Dummy-Session" );
}
