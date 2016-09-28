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

#include "RicAppendIntersectionBoxFeature.h"

#include "RimCase.h"
#include "RimIntersectionBox.h"
#include "RimIntersectionBoxCollection.h"
#include "RimView.h"
#include "RiuMainWindow.h"

#include "cafCmdExecCommandManager.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicAppendIntersectionBoxFeature, "RicAppendIntersectionBoxFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicAppendIntersectionBoxFeature::isCommandEnabled()
{
    RimIntersectionBoxCollection* coll = RicAppendIntersectionBoxFeature::intersectionBoxCollection();
    if (coll) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAppendIntersectionBoxFeature::onActionTriggered(bool isChecked)
{
    RimIntersectionBoxCollection* coll = RicAppendIntersectionBoxFeature::intersectionBoxCollection();

    if (coll)
    {
        RimIntersectionBox* intersectionBox = new RimIntersectionBox();
        intersectionBox->name = QString("Intersection Box");

        coll->appendIntersectionBox(intersectionBox);

        intersectionBox->setToDefaultSizeBox();

        coll->updateConnectedEditors();
        RiuMainWindow::instance()->selectAsCurrentItem(intersectionBox);

        RimView* rimView = NULL;
        coll->firstAncestorOrThisOfType(rimView);
        if (rimView)
        {
            rimView->showGridCells(false);
            rimView->scheduleCreateDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAppendIntersectionBoxFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/IntersectionBox16x16.png"));
    actionToSetup->setText("New Intersection Box");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIntersectionBoxCollection* RicAppendIntersectionBoxFeature::intersectionBoxCollection()
{
    RimIntersectionBoxCollection* intersectionBoxColl = nullptr;

    std::vector<caf::PdmObjectHandle*> selectedObjects;
    caf::SelectionManager::instance()->objectsByType(&selectedObjects);
    if (selectedObjects.size() == 1)
    {
        selectedObjects[0]->firstAncestorOrThisOfType(intersectionBoxColl);
    }

    return intersectionBoxColl;
}

