//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2019- Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include "cafToggleItemsOnFeature.h"

#include "cafToggleItemsFeatureImpl.h"

#include "cafSelectionManager.h"

#include <QAction>

namespace caf
{
CAF_CMD_SOURCE_INIT( ToggleItemsOnFeature, "cafToggleItemsOnFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ToggleItemsOnFeature::isCommandEnabled()
{
    return ToggleItemsFeatureImpl::isToggleCommandsAvailable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ToggleItemsOnFeature::onActionTriggered( bool isChecked )
{
    ToggleItemsFeatureImpl::setObjectToggleStateForSelection( ToggleItemsFeatureImpl::TOGGLE_ON );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ToggleItemsOnFeature::setupActionLook( QAction* actionToSetup )
{
    if ( ToggleItemsFeatureImpl::isToggleCommandsForSubItems() )
        actionToSetup->setText( "Sub Items On" );
    else
        actionToSetup->setText( "On" );

    actionToSetup->setIcon( QIcon( ":/cafCommandFeatures/ToggleOnL16x16.png" ) );
}

} // namespace caf