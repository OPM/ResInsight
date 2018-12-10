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

#include "RicLinkViewFeature.h"

#include "RiaApplication.h"

#include "RicLinkVisibleViewsFeature.h"

#include "Rim3dView.h"
#include "RimContourMapView.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimViewLinkerCollection.h"
#include "RimViewLinker.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicLinkViewFeature, "RicLinkViewFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicLinkViewFeature::isCommandEnabled()
{
    std::vector<caf::PdmUiItem*> allSelectedItems;
    std::vector<RimGridView*> selectedGridViews;
    std::vector<RimContourMapView*> selectedContourMaps;

    caf::SelectionManager::instance()->selectedItems(allSelectedItems);
    caf::SelectionManager::instance()->objectsByType(&selectedGridViews);
    caf::SelectionManager::instance()->objectsByType(&selectedContourMaps);
    size_t selectedRegularGridViews = selectedGridViews.size() - selectedContourMaps.size();

    if (selectedGridViews.size() > 1u && selectedRegularGridViews >= 1u && allSelectedItems.size() == selectedGridViews.size())
    {
        return true;
    }
    else
    {
        // Link only the active view to an existing view link collection.
        Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();
        if (!activeView) return false;

        RimProject* proj = RiaApplication::instance()->project();
        RimViewLinker* viewLinker = proj->viewLinkerCollection->viewLinker();

        if (!viewLinker) return false;

        RimViewController* viewController = activeView->viewController();

        if (viewController)
        {
            return false;
        }
        else if (!activeView->isMasterView())
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicLinkViewFeature::onActionTriggered(bool isChecked)
{
    std::vector<caf::PdmUiItem*> allSelectedItems;
    std::vector<RimGridView*> selectedGridViews;

    caf::SelectionManager::instance()->selectedItems(allSelectedItems);
    caf::SelectionManager::instance()->objectsByType(&selectedGridViews);

    if (selectedGridViews.size() > 1u && allSelectedItems.size() == selectedGridViews.size())
    {
        RicLinkVisibleViewsFeature::linkViews(selectedGridViews);
    }
    else
    {
        Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();
        RimGridView* gridView = dynamic_cast<RimGridView*>(activeView);
        if (gridView)
        {
            std::vector<RimGridView*> views;
            views.push_back(gridView);
            RicLinkVisibleViewsFeature::linkViews(views);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicLinkViewFeature::setupActionLook(QAction* actionToSetup)
{
    std::vector<RimGridView*> selectedGridViews;
    caf::SelectionManager::instance()->objectsByType(&selectedGridViews);
    if (selectedGridViews.size() > 1u)
    {
        actionToSetup->setText("Link Selected Views");
    }
    else
    {
        actionToSetup->setText("Link View");
    }
    actionToSetup->setIcon(QIcon(":/chain.png"));
}

