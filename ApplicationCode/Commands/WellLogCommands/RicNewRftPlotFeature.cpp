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

#include "RicNewRftPlotFeature.h"

#include "RicWellLogPlotCurveFeatureImpl.h"
#include "RicNewWellLogPlotFeatureImpl.h"

#include "RiaApplication.h"

#include "RigWellLogCurveData.h"

#include "RimProject.h"
#include "RimView.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellRftPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimRftPlotCollection.h"
#include "RimMainPlotCollection.h"

#include "RiuMainPlotWindow.h"

#include "cafSelectionManager.h"

#include <QAction>

#include <vector>
#include "RimEclipseWell.h"
#include "RiuSelectionManager.h"


CAF_CMD_SOURCE_INIT(RicNewRftPlotFeature, "RicNewRftPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewRftPlotFeature::isCommandEnabled()
{
    if (RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot()) return false;
    int branchIndex;
    return (selectedWellLogPlotTrack() != nullptr || selectedWellPath() != nullptr || selectedSimulationWell(&branchIndex) != nullptr) && caseAvailable();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewRftPlotFeature::onActionTriggered(bool isChecked)
{
    if (RicWellLogPlotCurveFeatureImpl::parentWellRftPlot()) return;

    RimProject* proj = RiaApplication::instance()->project();

    auto rftPlotColl = proj->mainPlotCollection()->rftPlotCollection();
    if (rftPlotColl)
    {
        RiuSelectionItem* selItem = RiuSelectionManager::instance()->selectedItem(RiuSelectionManager::RUI_TEMPORARY);
        RiuSimWellSelectionItem* simWellSelItem = dynamic_cast<RiuSimWellSelectionItem*>(selItem);
        QString plotName = QString("RFT: %1").arg(simWellSelItem != nullptr ? simWellSelItem->m_simWell->name : QString("(Unknown)"));

        auto plot = new RimWellRftPlot();
        rftPlotColl->rftPlots().push_back(plot);

        plot->setDescription(plotName);
        plot->loadDataAndUpdate();

        rftPlotColl->updateConnectedEditors();

        RiuMainPlotWindow* mainPlotWindow = RiaApplication::instance()->getOrCreateAndShowMainPlotWindow();
        if (mainPlotWindow)
        {
            mainPlotWindow->selectAsCurrentItem(plot);
            mainPlotWindow->setExpanded(plot, true);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewRftPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New RFT Plot");
    actionToSetup->setIcon(QIcon(":/SummaryPlot16x16.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogTrack* RicNewRftPlotFeature::selectedWellLogPlotTrack() const
{
    std::vector<RimWellLogTrack*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    return selection.size() > 0 ? selection[0] : NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath* RicNewRftPlotFeature::selectedWellPath() const
{
    std::vector<RimWellPath*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    return selection.size() > 0 ? selection[0] : NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseWell* RicNewRftPlotFeature::selectedSimulationWell(int * branchIndex) const
{
    RiuSelectionItem* selItem = RiuSelectionManager::instance()->selectedItem(RiuSelectionManager::RUI_TEMPORARY);
    RiuSimWellSelectionItem* simWellSelItem = dynamic_cast<RiuSimWellSelectionItem*>(selItem);
    if (simWellSelItem)
    {
        (*branchIndex) = static_cast<int>(simWellSelItem->m_branchIndex);
        return simWellSelItem->m_simWell;
    }
    else
    {
        std::vector<RimEclipseWell*> selection;
        caf::SelectionManager::instance()->objectsByType(&selection);
        (*branchIndex) = 0;
        return selection.size() > 0 ? selection[0] : NULL;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewRftPlotFeature::caseAvailable() const
{
    std::vector<RimCase*> cases;
    RiaApplication::instance()->project()->allCases(cases);

    return cases.size() > 0;
}
