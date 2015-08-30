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

#include "RicLinkVisibleViewsFeature.h"

#include "RiaApplication.h"

#include "RimManagedViewCollection.h"
#include "RimManagedViewConfig.h"
#include "RimProject.h"
#include "RimView.h"

#include "RiuMainWindow.h"

#include "cafPdmUiTreeView.h"

#include <QAction>
#include <QTreeView>

CAF_CMD_SOURCE_INIT(RicLinkVisibleViewsFeature, "RicLinkVisibleViewsFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicLinkVisibleViewsFeature::isCommandEnabled()
{
    RimProject* proj = RiaApplication::instance()->project();
    std::vector<RimView*> views;
    proj->allVisibleViews(views);

    if (views.size() > 1) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicLinkVisibleViewsFeature::onActionTriggered(bool isChecked)
{
    RimProject* proj = RiaApplication::instance()->project();
    std::vector<RimView*> views;
    proj->allVisibleViews(views);

    CVF_ASSERT(views.size() > 1);

    RimView* masterView = views[0];

    RimManagedViewCollection* managedViewCollection = masterView->managedViewCollection();
    for (size_t i = 1; i < views.size(); i++)
    {
        RimView* rimView = views[i];
        RimManagedViewConfig* viewConfig = new RimManagedViewConfig;
        viewConfig->managedView = rimView;
        managedViewCollection->managedViews.push_back(viewConfig);

        viewConfig->initAfterReadRecursively();
    }

    managedViewCollection->applyAllOperations();
    managedViewCollection->updateConnectedEditors();

    // Set managed view collection to selected and expanded in project tree
    caf::PdmUiTreeView* projTreeView = RiuMainWindow::instance()->projectTreeView();
    QModelIndex modIndex = projTreeView->findModelIndex(managedViewCollection);
    projTreeView->treeView()->setCurrentIndex(modIndex);

    projTreeView->treeView()->setExpanded(modIndex, true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicLinkVisibleViewsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Link Visible Views");
    actionToSetup->setIcon(QIcon(":/chain.png"));
}

