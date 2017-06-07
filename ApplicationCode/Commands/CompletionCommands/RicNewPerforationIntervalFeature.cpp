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

#include "WellPathCommands/RicWellPathsUnitSystemSettingsImpl.h"

#include "RiuMainWindow.h"

#include "RimPerforationInterval.h"
#include "RimPerforationCollection.h"
#include "RimWellPathCollection.h"

#include "cafSelectionManager.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicNewPerforationIntervalFeature, "RicNewPerforationIntervalFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewPerforationIntervalFeature::isCommandEnabled()
{
    return selectedPerforationCollection() != nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewPerforationIntervalFeature::onActionTriggered(bool isChecked)
{
    RimPerforationCollection* perforationCollection = selectedPerforationCollection();
    if (perforationCollection == nullptr) return;

    RimWellPath* wellPath;
    perforationCollection->firstAncestorOrThisOfTypeAsserted(wellPath);
    if (!RicWellPathsUnitSystemSettingsImpl::ensureHasUnitSystem(wellPath)) return;

    RimPerforationInterval* perforationInterval = new RimPerforationInterval;

    perforationCollection->appendPerforation(perforationInterval);

    RimWellPathCollection* wellPathCollection = nullptr;
    perforationCollection->firstAncestorOrThisOfType(wellPathCollection);
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
RimPerforationCollection* RicNewPerforationIntervalFeature::selectedPerforationCollection()
{
    RimPerforationCollection* objToFind = nullptr;
    
    caf::PdmUiItem* pdmUiItem = caf::SelectionManager::instance()->selectedItem();

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(pdmUiItem);
    if (objHandle)
    {
        objHandle->firstAncestorOrThisOfType(objToFind);
    }

    return objToFind;
}
