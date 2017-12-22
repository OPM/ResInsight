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

#include "RicNewFishbonesSubsAtMeasuredDepthFeature.h"

#include "RicNewFishbonesSubsFeature.h"
#include "WellPathCommands/RicWellPathsUnitSystemSettingsImpl.h"

#include "RimFishbonesCollection.h"
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

    if (!RicWellPathsUnitSystemSettingsImpl::ensureHasUnitSystem(wellPath)) return;
    
    RimFishbonesMultipleSubs* obj = new RimFishbonesMultipleSubs;
    wellPath->fishbonesCollection()->appendFishbonesSubs(obj);

    obj->setMeasuredDepthAndCount(wellPathSelItem->m_measuredDepth, 12.5, 13);


    RicNewFishbonesSubsFeature::askUserToSetUsefulScaling(wellPath->fishbonesCollection());

    wellPath->updateConnectedEditors();
    RiuMainWindow::instance()->selectAsCurrentItem(obj);

    RimProject* proj;
    wellPath->firstAncestorOrThisOfTypeAsserted(proj);
    proj->reloadCompletionTypeResultsInAllViews();
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
    actionToSetup->setIcon(QIcon(":/FishBoneGroup16x16.png"));
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
