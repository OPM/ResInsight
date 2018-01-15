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

#include "RicSetMasterViewFeature.h"

#include "RiaApplication.h"

#include "RimProject.h"
#include "Rim3dView.h"
#include "RimViewController.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"

#include "RiuMainWindow.h"

#include <QAction>
#include <QTreeView>

CAF_CMD_SOURCE_INIT(RicSetMasterViewFeature, "RicSetMasterViewFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSetMasterViewFeature::isCommandEnabled()
{
    RimGridView* activeView = RiaApplication::instance()->activeGridView();
    if (!activeView) return false;

    RimProject* proj = RiaApplication::instance()->project();
    RimViewLinker* viewLinker = activeView->assosiatedViewLinker();
    if (viewLinker && viewLinker->masterView() == activeView)
    {
        return false;
    }

    if (!proj->viewLinkerCollection()->viewLinker())
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSetMasterViewFeature::onActionTriggered(bool isChecked)
{
    RimGridView* activeView = RiaApplication::instance()->activeGridView();
    if (!activeView) return;

    RimProject* proj = RiaApplication::instance()->project();
    RimViewLinker* viewLinker = proj->viewLinkerCollection()->viewLinker();

    viewLinker->applyRangeFilterCollectionByUserChoice();

    RimGridView* previousMasterView = viewLinker->masterView();

    viewLinker->setMasterView(activeView);
    viewLinker->updateDependentViews();

    viewLinker->addDependentView(previousMasterView);

    proj->viewLinkerCollection.uiCapability()->updateConnectedEditors();
    proj->updateConnectedEditors();

    // Set managed view collection to selected and expanded in project tree
    RiuMainWindow::instance()->selectAsCurrentItem(viewLinker);
    RiuMainWindow::instance()->setExpanded(viewLinker);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSetMasterViewFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Set As Master View");
}

