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
#include "RimGridView.h"
#include "RimIntersectionBox.h"
#include "RimIntersectionCollection.h"

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
    RimGridView* activeView = RiaApplication::instance()->activeGridView();
    if (activeView)
    {
        RimIntersectionCollection* coll = activeView->crossSectionCollection();
        CVF_ASSERT(coll);

        RimIntersectionBox* intersectionBox = new RimIntersectionBox();
        intersectionBox->name               = QString("Y-slice (Intersection box)");

        coll->appendIntersectionBoxAndUpdate(intersectionBox);

        cvf::Vec3d domainCoord = activeView->viewer()->lastPickPositionInDomainCoords();
        intersectionBox->setToDefaultSizeSlice(RimIntersectionBox::PLANE_STATE_Y, domainCoord);

        coll->updateConnectedEditors();
        RiuMainWindow::instance()->selectAsCurrentItem(intersectionBox);

        RimGridView* rimView = nullptr;
        coll->firstAncestorOrThisOfType(rimView);
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
    actionToSetup->setIcon(QIcon(":/IntersectionYPlane16x16.png"));
    actionToSetup->setText("Y-slice Intersection Box");
}
