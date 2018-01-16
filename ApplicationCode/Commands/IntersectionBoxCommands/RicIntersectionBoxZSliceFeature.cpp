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

#include "RicIntersectionBoxZSliceFeature.h"

#include "RiaApplication.h"

#include "RimCase.h"
#include "RimIntersectionBox.h"
#include "RimIntersectionCollection.h"
#include "RimGridView.h"

#include "RiuMainWindow.h"
#include "RiuViewer.h"

#include "cafCmdExecCommandManager.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicIntersectionBoxZSliceFeature, "RicIntersectionBoxZSliceFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicIntersectionBoxZSliceFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicIntersectionBoxZSliceFeature::onActionTriggered(bool isChecked)
{
    RimGridView* activeView = RiaApplication::instance()->activeGridView();
    if (activeView)
    {
        RimIntersectionCollection* coll = activeView->crossSectionCollection();
        CVF_ASSERT(coll);

        RimIntersectionBox* intersectionBox = new RimIntersectionBox();
        intersectionBox->name = QString("Z-slice (Intersection box)");

        coll->appendIntersectionBox(intersectionBox);
        cvf::Vec3d domainCoord = activeView->viewer()->lastPickPositionInDomainCoords();
        intersectionBox->setToDefaultSizeSlice(RimIntersectionBox::PLANE_STATE_Z, domainCoord);

        coll->updateConnectedEditors();
        RiuMainWindow::instance()->selectAsCurrentItem(intersectionBox);

        RimGridView* rimView = NULL;
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
void RicIntersectionBoxZSliceFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/IntersectionZPlane16x16.png"));
    actionToSetup->setText("Z-slice Intersection Box");
}

