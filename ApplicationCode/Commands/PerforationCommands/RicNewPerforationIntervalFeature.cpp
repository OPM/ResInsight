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

#include "RicNewPerforationIntervalFeature.h"

#include "RiuMainWindow.h"

#include "RimPerforationInterval.h"
#include "RimPerforationCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "cafSelectionManager.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicNewPerforationIntervalFeature, "RicNewPerforationIntervalFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewPerforationIntervalFeature::isCommandEnabled()
{
    return selectedWellPath() != nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewPerforationIntervalFeature::onActionTriggered(bool isChecked)
{
    RimWellPath* wellPath = selectedWellPath();
    if (wellPath == nullptr) return;

    RimPerforationInterval* perforationInterval = new RimPerforationInterval;

    wellPath->perforationIntervalCollection()->appendPerforation(perforationInterval);

    RimWellPathCollection* wellPathCollection = nullptr;
    wellPath->firstAncestorOrThisOfType(wellPathCollection);
    if (!wellPathCollection) return;

    wellPathCollection->uiCapability()->updateConnectedEditors();
    wellPathCollection->scheduleGeometryRegenAndRedrawViews();

    RiuMainWindow::instance()->selectAsCurrentItem(perforationInterval);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewPerforationIntervalFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Perforation Interval");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath * RicNewPerforationIntervalFeature::selectedWellPath()
{
    RimWellPath* wellPath = nullptr;
    
    caf::PdmUiItem* pdmUiItem = caf::SelectionManager::instance()->selectedItem();

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(pdmUiItem);
    if (objHandle)
    {
        objHandle->firstAncestorOrThisOfType(wellPath);
    }

    return wellPath;
}
