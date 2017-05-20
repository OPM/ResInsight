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

#include "RiuMainWindow.h"

#include "cafSelectionManager.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicNewFishbonesSubsFeature, "RicNewFishbonesSubsFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsFeature::onActionTriggered(bool isChecked)
{
    RimFishbonesCollection* fishbonesCollection = selectedFishbonesCollection();
    CVF_ASSERT(fishbonesCollection);

    RimFishbonesMultipleSubs* obj = new RimFishbonesMultipleSubs;
    obj->setName(QString("Fishbones Subs (%1)").arg(fishbonesCollection->fishbonesSubs.size()));
    fishbonesCollection->fishbonesSubs.push_back(obj);

    fishbonesCollection->updateConnectedEditors();
    RiuMainWindow::instance()->selectAsCurrentItem(obj);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFishbonesCollection* RicNewFishbonesSubsFeature::selectedFishbonesCollection()
{
    RimFishbonesCollection* objToFind = nullptr;
    
    caf::PdmUiItem* pdmUiItem = caf::SelectionManager::instance()->selectedItem();

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(pdmUiItem);
    if (objHandle)
    {
        objHandle->firstAncestorOrThisOfType(objToFind);
    }

    return objToFind;
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
    if (selectedFishbonesCollection())
    {
        return true;
    }

    return false;
}
