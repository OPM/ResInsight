/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicNewFishbonesSubsFeature.h"

#include "RimFishbonesMultipleSubs.h"
#include "RimFishbonesCollection.h"
#include "RimWellPath.h"

#include "RiuMainWindow.h"

#include "cafSelectionManager.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicNewFishbonesSubsFeature, "RicNewFishbonesSubsFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsFeature::onActionTriggered(bool isChecked)
{
    RimWellPath* wellPath = selectedWellPath();
    CVF_ASSERT(wellPath);

    RimFishbonesMultipleSubs* obj = new RimFishbonesMultipleSubs;
    obj->setName(QString("Fishbones Subs (%1)").arg(wellPath->fishbonesCollection()->fishbonesSubs.size()));
    wellPath->fishbonesCollection()->fishbonesSubs.push_back(obj);

    wellPath->updateConnectedEditors();
    RiuMainWindow::instance()->selectAsCurrentItem(obj);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath* RicNewFishbonesSubsFeature::selectedWellPath()
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsFeature::setupActionLook(QAction* actionToSetup)
{
    //actionToSetup->setIcon(QIcon(":/FractureSymbol16x16.png"));
    actionToSetup->setText("New Fishbones Subs Definition");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewFishbonesSubsFeature::isCommandEnabled()
{
    if (selectedWellPath())
    {
        return true;
    }

    return false;
}
