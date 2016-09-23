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

#include "RicIntersectionBoxYSliceFeature.h"

#include "RiaApplication.h"

#include "RimCase.h"
#include "RimIntersectionBox.h"
#include "RimIntersectionBoxCollection.h"
#include "RimView.h"

#include "RiuMainWindow.h"
#include "RiuViewer.h"

#include "cafCmdExecCommandManager.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicIntersectionBoxYSliceFeature, "RicIntersectionBoxYSliceFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicIntersectionBoxYSliceFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicIntersectionBoxYSliceFeature::onActionTriggered(bool isChecked)
{
    RimView* activeView = RiaApplication::instance()->activeReservoirView();
    if (activeView)
    {
        RimIntersectionBoxCollection* coll = activeView->intersectionBoxCollection();
        CVF_ASSERT(coll);

        RimIntersectionBox* intersectionBox = new RimIntersectionBox();
        intersectionBox->name = QString("Y-slice (Intersection box)");

        RimCase* rimCase = NULL;
        coll->firstAnchestorOrThisOfType(rimCase);
        if (rimCase)
        {
            intersectionBox->setModelBoundingBox(rimCase->activeCellsBoundingBox());

            cvf::Vec3d domainCoord = activeView->viewer()->lastPickPositionInDomainCoords();

            if (!domainCoord.isUndefined())
            {
                intersectionBox->setYSlice(domainCoord.y());
            }
            else
            {
                intersectionBox->setYSlice(rimCase->activeCellsBoundingBox().center().y());
            }
        }

        coll->appendIntersectionBox(intersectionBox);

        coll->updateConnectedEditors();
        RiuMainWindow::instance()->selectAsCurrentItem(intersectionBox);

        RimView* rimView = NULL;
        coll->firstAnchestorOrThisOfType(rimView);
        if (rimView)
        {
            rimView->showGridCells(false);
            RiuMainWindow::instance()->refreshDrawStyleActions();

            rimView->scheduleCreateDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicIntersectionBoxYSliceFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/IntersectionBox16x16.png"));
    actionToSetup->setText("Y-slice Intersection Box");
}

