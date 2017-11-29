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

#include "RicHideIntersectionFeature.h"

#include "RiaApplication.h"

#include "RimIntersection.h"
#include "RimView.h"

#include "RiuSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicHideIntersectionFeature, "RicHideIntersectionFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicHideIntersectionFeature::isCommandEnabled()
{
    RimView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return false;

    RiuSelectionManager* riuSelManager = RiuSelectionManager::instance();
    RiuSelectionItem* selItem = riuSelManager->selectedItem(RiuSelectionManager::RUI_TEMPORARY);

    RiuGeneralSelectionItem* generalSelectionItem = static_cast<RiuGeneralSelectionItem*>(selItem);
    if (!generalSelectionItem) return false;

    RimIntersection* intersection = dynamic_cast<RimIntersection*>(generalSelectionItem->m_object);
    if (intersection)
    {
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicHideIntersectionFeature::onActionTriggered(bool isChecked)
{
    RimView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return;

    RiuSelectionManager* riuSelManager = RiuSelectionManager::instance();
    RiuSelectionItem* selItem = riuSelManager->selectedItem(RiuSelectionManager::RUI_TEMPORARY);

    RiuGeneralSelectionItem* generalSelectionItem = static_cast<RiuGeneralSelectionItem*>(selItem);
    if (!generalSelectionItem) return;

    RimIntersection* intersection = dynamic_cast<RimIntersection*>(generalSelectionItem->m_object);
    if (intersection)
    {
        intersection->isActive = false;
        intersection->updateConnectedEditors();

        activeView->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicHideIntersectionFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Hide Intersection");
    actionToSetup->setIcon(QIcon(":/IntersectionXPlane16x16.png"));
}
