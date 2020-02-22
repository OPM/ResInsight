/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicToggleItemsOffFeature.h"

#include "RicToggleItemsFeatureImpl.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicToggleItemsOffFeature, "RicToggleItemsOffFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicToggleItemsOffFeature::isCommandEnabled()
{
    return RicToggleItemsFeatureImpl::isToggleCommandsAvailable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicToggleItemsOffFeature::onActionTriggered( bool isChecked )
{
    RicToggleItemsFeatureImpl::setObjectToggleStateForSelection( RicToggleItemsFeatureImpl::TOGGLE_OFF );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicToggleItemsOffFeature::setupActionLook( QAction* actionToSetup )
{
    if ( RicToggleItemsFeatureImpl::isToggleCommandsForSubItems() )
        actionToSetup->setText( "Sub Items Off" );
    else
        actionToSetup->setText( "Off" );

    actionToSetup->setIcon( QIcon( ":/ToggleOff16x16.png" ) );
}
