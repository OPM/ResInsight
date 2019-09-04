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

#include "RimEclipseContourMapView.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimViewController.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmUiPropertyViewDialog.h"

#include <QAction>
#include <QMessageBox>
#include <QTreeView>

CAF_CMD_SOURCE_INIT(RicLinkVisibleViewsFeature, "RicLinkVisibleViewsFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicLinkVisibleViewsFeature::isCommandEnabled()
{
    RimProject*               proj = RiaApplication::instance()->project();
    std::vector<Rim3dView*>   visibleViews;
    std::vector<RimGridView*> linkedviews;
    std::vector<RimGridView*> visibleGridViews;

    proj->allVisibleViews(visibleViews);
    for (Rim3dView* view : visibleViews)
    {
        RimGridView* gridView = dynamic_cast<RimGridView*>(view);
        if (gridView) visibleGridViews.push_back(gridView);
    }

    if (proj->viewLinkerCollection() && proj->viewLinkerCollection()->viewLinker())
    {
        proj->viewLinkerCollection()->viewLinker()->allViews(linkedviews);
    }

    if (visibleGridViews.size() >= 2 && (linkedviews.size() < visibleGridViews.size()))
    {
        std::vector<RimGridView*> views;
        findNotLinkedVisibleViews(views);
        RicLinkVisibleViewsFeatureUi testUi;
        testUi.setViews(views);
        return !testUi.masterViewCandidates().empty();
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicLinkVisibleViewsFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimGridView*> views;
    findNotLinkedVisibleViews(views);

    linkViews(views);
    return;
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
void RicLinkVisibleViewsFeature::allLinkedViews(std::vector<RimGridView*>& views)
{
    RimProject* proj = RiaApplication::instance()->project();
    if (proj->viewLinkerCollection()->viewLinker())
    {
        proj->viewLinkerCollection()->viewLinker()->allViews(views);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicLinkVisibleViewsFeature::findNotLinkedVisibleViews(std::vector<RimGridView*>& views)
{
    RimProject* proj = RiaApplication::instance()->project();

    std::vector<RimGridView*> alreadyLinkedViews;
    allLinkedViews(alreadyLinkedViews);

    std::vector<RimGridView*> visibleViews;
    proj->allVisibleGridViews(visibleViews);

    for (size_t i = 0; i < visibleViews.size(); i++)
    {
        bool isLinked = false;
        for (size_t j = 0; j < alreadyLinkedViews.size(); j++)
        {
            if (visibleViews[i] == alreadyLinkedViews[j])
            {
                isLinked = true;
            }
        }

        if (!isLinked)
        {
            views.push_back(visibleViews[i]);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicLinkVisibleViewsFeature::linkViews(std::vector<RimGridView*>& views)
{
    RimProject*    proj       = RiaApplication::instance()->project();
    RimViewLinker* viewLinker = proj->viewLinkerCollection->viewLinker();

    std::vector<RimGridView*> masterCandidates;
    for (RimGridView* view : views)
    {
        if (dynamic_cast<RimEclipseContourMapView*>(view) == nullptr)
        {
            masterCandidates.push_back(view);
        }
    }

    if (!viewLinker)
    {
        // Create a new view linker

        if (views.size() < 2)
        {
            return;
        }
        CVF_ASSERT(!masterCandidates.empty());

        RimGridView* masterView = masterCandidates.front();
        if (masterCandidates.size() > 1u)
        {
            RicLinkVisibleViewsFeatureUi featureUi;
            featureUi.setViews(masterCandidates);

            caf::PdmUiPropertyViewDialog propertyDialog(nullptr, &featureUi, "Select Master View", "");
            propertyDialog.setWindowIcon(QIcon(":/chain.png"));
            if (propertyDialog.exec() != QDialog::Accepted) return;

            masterView = featureUi.masterView();
        }
        viewLinker                               = new RimViewLinker;
        proj->viewLinkerCollection()->viewLinker = viewLinker;
        viewLinker->setMasterView(masterView);
    }

    for (size_t i = 0; i < views.size(); i++)
    {
        RimGridView* rimView = views[i];
        if (rimView == viewLinker->masterView()) continue;

        viewLinker->addDependentView(rimView);
    }

    viewLinker->updateDependentViews();

    viewLinker->updateUiNameAndIcon();

    proj->viewLinkerCollection.uiCapability()->updateConnectedEditors();
    proj->updateConnectedEditors();

    Riu3DMainWindowTools::setExpanded(proj->viewLinkerCollection());
}
