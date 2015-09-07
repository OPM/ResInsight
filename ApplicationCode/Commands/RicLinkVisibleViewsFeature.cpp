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

#include "RicLinkVisibleViewsFeatureUi.h"

#include "RimLinkedView.h"
#include "RimProject.h"
#include "RimView.h"
#include "RimViewLinker.h"

#include "RiuMainWindow.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafPdmUiTreeView.h"

#include <QAction>
#include <QTreeView>
#include <QMessageBox>


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
    findNotLinkedVisibleViews(views);
    if (views.size() < 2)
    {
        QMessageBox::warning(RiuMainWindow::instance(), "Link Visible Views", "Less than two views are available for linking. Please open at least two not linked views before creating a new linked views group.");

        return;
    }

    RicLinkVisibleViewsFeatureUi featureUi;
    featureUi.setViews(views);

    caf::PdmUiPropertyViewDialog propertyDialog(NULL, &featureUi, "Select Master View", "");
    propertyDialog.setWindowIcon(QIcon(":/chain.png"));
    if (propertyDialog.exec() != QDialog::Accepted) return;

    RimView* masterView = featureUi.masterView();
    RimViewLinker* linkedViews = new RimViewLinker;
    linkedViews->setMainView(masterView);

    for (size_t i = 0; i < views.size(); i++)
    {
        RimView* rimView = views[i];
        if (rimView == masterView) continue;

        RimLinkedView* viewConfig = new RimLinkedView;
        viewConfig->setManagedView(rimView);

        linkedViews->linkedViews.push_back(viewConfig);

        viewConfig->initAfterReadRecursively();
        viewConfig->updateViewChanged();
    }

    proj->linkedViews.push_back(linkedViews);
    proj->linkedViews.uiCapability()->updateConnectedEditors();

    linkedViews->applyAllOperations();
    proj->updateConnectedEditors();

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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicLinkVisibleViewsFeature::allLinkedViews(std::vector<RimView*>& views)
{
    RimProject* proj = RiaApplication::instance()->project();
    for (size_t i = 0; i < proj->linkedViews().size(); i++)
    {
        RimViewLinker* linkedViews = proj->linkedViews()[i];
        linkedViews->allViews(views);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicLinkVisibleViewsFeature::findNotLinkedVisibleViews(std::vector<RimView*> &views)
{
    RimProject* proj = RiaApplication::instance()->project();

    std::vector<RimView*> alreadyLinkedViews;
    allLinkedViews(alreadyLinkedViews);

    std::vector<RimView*> visibleViews;
    proj->allVisibleViews(visibleViews);

    bool anyAlreadyLinkedViews = false;
    for (size_t i = 0; i < visibleViews.size(); i++)
    {
        bool isLinked = false;
        for (size_t j = 0; j < alreadyLinkedViews.size(); j++)
        {
            if (visibleViews[i] == alreadyLinkedViews[j])
            {
                anyAlreadyLinkedViews = true;
                isLinked = true;
            }
        }

        if (!isLinked)
        {
            views.push_back(visibleViews[i]);
        }
    }

    if (anyAlreadyLinkedViews)
    {
        QMessageBox::warning(RiuMainWindow::instance(), "Link Visible Views",
            "Detected one or more visible view(s) already part of a Linked View Group.\nThese views were removed from the list views to be linked.");
    }
}

