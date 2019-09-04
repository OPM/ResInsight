/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicUnLinkViewFeature.h"

#include "RiaApplication.h"

#include "Rim3dView.h"
#include "RimProject.h"
#include "RimViewController.h"
#include "RimViewLinker.h"

#include "cafCmdFeatureManager.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicUnLinkViewFeature, "RicUnLinkViewFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicUnLinkViewFeature::isCommandEnabled()
{
    Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return false;

    RimViewController* viewController = activeView->viewController();

    if (viewController)
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicUnLinkViewFeature::onActionTriggered(bool isChecked)
{
    Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return;

    RimViewController* viewController = activeView->viewController();
    viewController->applyRangeFilterCollectionByUserChoice();

    caf::SelectionManager::instance()->setSelectedItem(viewController);
    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature("RicDeleteItemFeature");
    if (feature)
    {
        feature->action()->trigger();

        return;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicUnLinkViewFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Unlink View");
}
