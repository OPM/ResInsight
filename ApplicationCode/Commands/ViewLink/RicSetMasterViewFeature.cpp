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
#include "RimView.h"
#include "RimViewLink.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"
#include "RiuMainWindow.h"

#include "cafPdmUiTreeView.h"

#include <QAction>
#include <QTreeView>

CAF_CMD_SOURCE_INIT(RicSetMasterViewFeature, "RicSetMasterViewFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSetMasterViewFeature::isCommandEnabled()
{
    RimView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return false;

    RimProject* proj = RiaApplication::instance()->project();
    RimViewLinker* viewLinker = proj->findViewLinkerFromView(activeView);
    if (viewLinker && viewLinker->mainView() == activeView)
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
    RimView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return;

    RimProject* proj = RiaApplication::instance()->project();
    RimViewLinker* viewLinker = proj->viewLinkerCollection()->viewLinker();

    RimView* previousMasterView = viewLinker->mainView();

    RimViewLink* previousViewLink = RimViewLinker::viewLinkForView(activeView);
    if (previousViewLink)
    {
        size_t indexToErase = cvf::UNDEFINED_SIZE_T;
        for (size_t i = 0; i < viewLinker->viewLinks.size(); i++)
        {
            if (viewLinker->viewLinks()[i] == previousViewLink)
            {
                indexToErase = i;
            }
        }

        if (indexToErase != cvf::UNDEFINED_SIZE_T)
        {
            viewLinker->viewLinks().erase(indexToErase);
        }

        delete previousViewLink;
    }

    viewLinker->removeOverrides();
    viewLinker->setMainView(activeView);

    if (previousMasterView)
    {
        RimViewLink* viewLink = new RimViewLink;
        viewLink->setManagedView(previousMasterView);

        viewLinker->viewLinks.push_back(viewLink);

        viewLink->initAfterReadRecursively();
        viewLink->updateOptionSensitivity();
        viewLink->updateUiIconFromActiveState();
    }

    viewLinker->applyAllOperations();
    proj->viewLinkerCollection.uiCapability()->updateConnectedEditors();
    proj->updateConnectedEditors();

    // Set managed view collection to selected and expanded in project tree
    caf::PdmUiTreeView* projTreeView = RiuMainWindow::instance()->projectTreeView();
    QModelIndex modIndex = projTreeView->findModelIndex(viewLinker);
    projTreeView->treeView()->setCurrentIndex(modIndex);

    projTreeView->treeView()->setExpanded(modIndex, true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSetMasterViewFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Set As Master View");
}

