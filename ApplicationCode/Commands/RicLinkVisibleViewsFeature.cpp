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

#include "RimLinkedViews.h"
#include "RimManagedViewConfig.h"
#include "RimProject.h"
#include "RimView.h"

#include "RiuMainWindow.h"

#include "cafPdmUiTreeView.h"

#include <QAction>
#include <QTreeView>
#include "RicLinkVisibleViewsFeatureUi.h"
#include "cafPdmUiPropertyViewDialog.h"

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

    RicLinkVisibleViewsFeatureUi featureUi;
    featureUi.setViews(views);

    caf::PdmUiPropertyViewDialog propertyDialog(NULL, &featureUi, "New View Group", "");
    propertyDialog.setWindowIcon(QIcon(":/chain.png"));
    if (propertyDialog.exec() != QDialog::Accepted) return;

    RimView* masterView = featureUi.masterView();
    RimLinkedViews* linkedViews = new RimLinkedViews;
    linkedViews->setMainView(masterView);

    for (size_t i = 0; i < views.size(); i++)
    {
        RimView* rimView = views[i];
        if (rimView == masterView) continue;

        RimManagedViewConfig* viewConfig = new RimManagedViewConfig;
        viewConfig->setManagedView(rimView);

        linkedViews->viewConfigs.push_back(viewConfig);

        viewConfig->initAfterReadRecursively();
        viewConfig->updateViewChanged();
    }

    proj->linkedViews.push_back(linkedViews);
    proj->linkedViews.uiCapability()->updateConnectedEditors();

    linkedViews->applyAllOperations();
    linkedViews->updateConnectedEditors();

    // Set managed view collection to selected and expanded in project tree
    caf::PdmUiTreeView* projTreeView = RiuMainWindow::instance()->projectTreeView();
    QModelIndex modIndex = projTreeView->findModelIndex(linkedViews);
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

