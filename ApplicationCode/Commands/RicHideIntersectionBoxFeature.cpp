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

#include "RicHideIntersectionBoxFeature.h"

#include "RiaApplication.h"

#include "RimIntersectionBox.h"
#include "Rim3dView.h"

#include "Riu3dSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicHideIntersectionBoxFeature, "RicHideIntersectionBoxFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicHideIntersectionBoxFeature::isCommandEnabled()
{
    Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return false;

    Riu3dSelectionManager* riuSelManager = Riu3dSelectionManager::instance();
    RiuSelectionItem* selItem = riuSelManager->selectedItem(Riu3dSelectionManager::RUI_TEMPORARY);

    RiuGeneralSelectionItem* generalSelectionItem = static_cast<RiuGeneralSelectionItem*>(selItem);
    if (!generalSelectionItem) return false;

    RimIntersectionBox* intersectionBox = dynamic_cast<RimIntersectionBox*>(generalSelectionItem->m_object);
    if (intersectionBox)
    {
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicHideIntersectionBoxFeature::onActionTriggered(bool isChecked)
{
    Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return;

    Riu3dSelectionManager* riuSelManager = Riu3dSelectionManager::instance();
    RiuSelectionItem* selItem = riuSelManager->selectedItem(Riu3dSelectionManager::RUI_TEMPORARY);

    RiuGeneralSelectionItem* generalSelectionItem = static_cast<RiuGeneralSelectionItem*>(selItem);
    if (!generalSelectionItem) return;

    RimIntersectionBox* intersectionBox = dynamic_cast<RimIntersectionBox*>(generalSelectionItem->m_object);
    if (intersectionBox)
    {
        intersectionBox->isActive = false;
        intersectionBox->updateConnectedEditors();

        activeView->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicHideIntersectionBoxFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Hide Intersection Box");
    actionToSetup->setIcon(QIcon(":/IntersectionBox16x16.png"));
}
