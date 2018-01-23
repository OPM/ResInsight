/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicNewWellPathFractureAtPosFeature.h"

#include "RiaApplication.h"

#include "RicNewWellPathFractureFeature.h"

#include "RimProject.h"

#include "RiuSelectionManager.h"

#include <QAction>



CAF_CMD_SOURCE_INIT(RicNewWellPathFractureAtPosFeature, "RicNewWellPathFractureAtPosFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathFractureAtPosFeature::onActionTriggered(bool isChecked)
{
    RimProject* proj = RiaApplication::instance()->project();
    if (proj->allFractureTemplates().empty()) return;

    RiuSelectionManager* riuSelManager = RiuSelectionManager::instance();
    RiuSelectionItem* selItem = riuSelManager->selectedItem(RiuSelectionManager::RUI_TEMPORARY);

    RiuWellPathSelectionItem* wellPathItem = dynamic_cast<RiuWellPathSelectionItem*>(selItem);
    if (!wellPathItem) return;

    RimWellPath* wellPath = wellPathItem->m_wellpath;
    if (!wellPath) return;

    RicNewWellPathFractureFeature::addFracture(wellPath, wellPathItem->m_measuredDepth);
}
 
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathFractureAtPosFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/FractureSymbol16x16.png"));
    actionToSetup->setText("New Fracture");
}
 
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellPathFractureAtPosFeature::isCommandEnabled()
{
    RimProject* proj = RiaApplication::instance()->project();
    if (proj->allFractureTemplates().empty()) return false;

    return true;
}
