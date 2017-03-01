/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RicShowContributingWellsFeature.h"

#include "RiaApplication.h"
#include "RimEclipseResultCase.h"
#include "RimView.h"

#include "cafCmdFeatureManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicShowContributingWellsFeature, "RicShowContributingWellsFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicShowContributingWellsFeature::isCommandEnabled()
{
    RimView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return false;

    RimEclipseResultCase* eclCase = nullptr;
    activeView->firstAncestorOrThisOfType(eclCase);
    if (eclCase)
    {
        std::vector<RimFlowDiagSolution*> flowSols = eclCase->flowDiagSolutions();
        if (flowSols.size() > 0)
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowContributingWellsFeature::onActionTriggered(bool isChecked)
{
    // First, shot the well allocation plot
    // Then, use the feature to show contributing wells as this is based on the previous feature

    std::vector<std::string> commandIds;
    commandIds.push_back("RicShowWellAllocationPlotFeature");
    commandIds.push_back("RicShowContributingWellsFromPlotFeature");

    for (auto commandId : commandIds)
    {
        auto* feature = caf::CmdFeatureManager::instance()->getCommandFeature(commandId);
        if (feature)
        {
            feature->actionTriggered(false);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowContributingWellsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/new_icon16x16.png"));
    actionToSetup->setText("Show Contributing Wells");
}
