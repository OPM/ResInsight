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

#include "RicToggleItemsFeature.h"

#include "RicToggleItemsFeatureImpl.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicToggleItemsFeature, "RicToggleItemsFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicToggleItemsFeature::isCommandEnabled() 
{
    return RicToggleItemsFeatureImpl::isToggleCommandsAvailable();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicToggleItemsFeature::onActionTriggered(bool isChecked)
{
    RicToggleItemsFeatureImpl::setObjectToggleStateForSelection(RicToggleItemsFeatureImpl::TOGGLE_SUBITEMS);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicToggleItemsFeature::setupActionLook(QAction* actionToSetup)
{
    if (RicToggleItemsFeatureImpl::isToggleCommandsForSubItems())
        actionToSetup->setText("Toggle Sub Items");
    else
        actionToSetup->setText("Toggle");
}
