/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicNewFishbonesSubsAtMeasuredDepthFeature.h"

#include "RimFishbonesMultipleSubs.h"
#include "RimProject.h"
#include "RimWellPath.h"

#include "RiuMainWindow.h"
#include "RiuSelectionManager.h"

#include "cafSelectionManager.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicNewFishbonesSubsAtMeasuredDepthFeature, "RicNewFishbonesSubsAtMeasuredDepthFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsAtMeasuredDepthFeature::onActionTriggered(bool isChecked)
{
    RiuWellPathSelectionItem* wellPathSelItem = wellPathSelectionItem();
    CVF_ASSERT(wellPathSelItem);

    RimWellPath* wellPath = wellPathSelItem->m_wellpath;
    CVF_ASSERT(wellPath);
    
    RimFishbonesMultipleSubs* obj = new RimFishbonesMultipleSubs;
    wellPath->fishbonesSubs.push_back(obj);

    obj->setName(QString("Fishbones Subs (%1)").arg(wellPath->fishbonesSubs.size()));
    int integerValue = wellPathSelItem->m_measuredDepth;
    obj->setMeasuredDepthAndCount(integerValue, 24, 5);

    wellPath->updateConnectedEditors();
    RiuMainWindow::instance()->selectAsCurrentItem(obj);

    RimProject* proj;
    wellPath->firstAncestorOrThisOfTypeAsserted(proj);
    proj->createDisplayModelAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellPathSelectionItem* RicNewFishbonesSubsAtMeasuredDepthFeature::wellPathSelectionItem()
{
    RiuSelectionManager* riuSelManager = RiuSelectionManager::instance();
    RiuSelectionItem* selItem = riuSelManager->selectedItem(RiuSelectionManager::RUI_TEMPORARY);

    RiuWellPathSelectionItem* wellPathItem = dynamic_cast<RiuWellPathSelectionItem*>(selItem);

    return wellPathItem;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsAtMeasuredDepthFeature::setupActionLook(QAction* actionToSetup)
{
    //actionToSetup->setIcon(QIcon(":/FractureSymbol16x16.png"));
    actionToSetup->setText("New Fishbones Subs Definition");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewFishbonesSubsAtMeasuredDepthFeature::isCommandEnabled()
{
    if (wellPathSelectionItem())
    {
        return true;
    }

    return false;
}
