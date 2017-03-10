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

#include "RimSummaryPlotCollection.h"
#include "RimWellPathCollection.h"

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

        RimSummaryPlotCollection* summaryPlotColl = dynamic_cast<RimSummaryPlotCollection*>(item);
        if (summaryPlotColl)
        {
            summaryPlotColl->summaryPlots.deleteAllChildObjects();

            summaryPlotColl->updateConnectedEditors();
        }

        RimWellPathCollection* wellPathColl = dynamic_cast<RimWellPathCollection*>(item);
        if (wellPathColl)
        {
            wellPathColl->deleteAllWellPaths();

            wellPathColl->updateConnectedEditors();
            wellPathColl->scheduleGeometryRegenAndRedrawViews();
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
    RimSummaryPlotCollection* summaryPlotColl = dynamic_cast<RimSummaryPlotCollection*>(uiItem);
    if (summaryPlotColl && summaryPlotColl->summaryPlots().size() > 0)
    {
        return true;
    }

    RimWellPathCollection* wellPathColl = dynamic_cast<RimWellPathCollection*>(uiItem);
    if (wellPathColl && wellPathColl->wellPaths().size() > 0)
    {
        return true;
    }

    return false;
}
