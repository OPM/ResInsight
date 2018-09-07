/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicDeleteSubItemsFeature.h"

#include "RimProject.h"
#include "RimSummaryPlotCollection.h"
#include "RimWellPathCollection.h"
#include "RimWellPathFractureCollection.h"

#include "cafPdmUiItem.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicDeleteSubItemsFeature, "RicDeleteSubItemsFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeleteSubItemsFeature::isCommandEnabled()
{
    std::vector<caf::PdmUiItem*> items;
    caf::SelectionManager::instance()->selectedItems(items);

    if (items.empty()) return false;

    for (auto* item : items)
    {
        if (!RicDeleteSubItemsFeature::hasDeletableSubItems(item)) return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteSubItemsFeature::onActionTriggered(bool isChecked)
{
    std::vector<caf::PdmUiItem*> items;
    caf::SelectionManager::instance()->selectedItems(items);

    CVF_ASSERT(items.size() > 0);

    for (auto item : items)
    {
        if (!RicDeleteSubItemsFeature::hasDeletableSubItems(item)) continue;

        {
            auto collection = dynamic_cast<RimSummaryPlotCollection*>(item);
            if (collection)
            {
                collection->summaryPlots.deleteAllChildObjects();

                collection->updateConnectedEditors();
            }
        }

        {
            auto collection = dynamic_cast<RimWellPathCollection*>(item);
            if (collection)
            {
                collection->deleteAllWellPaths();

                collection->updateConnectedEditors();
                collection->scheduleRedrawAffectedViews();
            }
        }

        {
            auto collection = dynamic_cast<RimWellPathFractureCollection*>(item);
            if (collection)
            {
                collection->deleteFractures();

                collection->updateConnectedEditors();

                RimProject* proj = nullptr;
                collection->firstAncestorOrThisOfType(proj);
                if (proj) proj->reloadCompletionTypeResultsInAllViews();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteSubItemsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Delete Sub Items");
    actionToSetup->setIcon(QIcon(":/Erase.png"));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeleteSubItemsFeature::hasDeletableSubItems(caf::PdmUiItem* uiItem)
{
    {
        auto collection = dynamic_cast<RimSummaryPlotCollection*>(uiItem);
        if (collection && !collection->summaryPlots().empty())
        {
            return true;
        }
    }

    {
        auto collection = dynamic_cast<RimWellPathCollection*>(uiItem);
        if (collection && !collection->wellPaths().empty())
        {
            return true;
        }
    }

    {
        auto collection = dynamic_cast<RimWellPathFractureCollection*>(uiItem);
        if (collection && !collection->allFractures().empty())
        {
            return true;
        }
    }

    return false;
}
