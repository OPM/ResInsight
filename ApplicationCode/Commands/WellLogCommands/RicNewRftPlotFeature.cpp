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
#include "RimSimWellInView.h"
#include "RimView.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellRftPlot.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimRftPlotCollection.h"
#include "RimMainPlotCollection.h"
#include "RimEclipseResultCase.h"

#include "RiuMainPlotWindow.h"
#include "RiuSelectionManager.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

#include <vector>


CAF_CMD_SOURCE_INIT(RicNewRftPlotFeature, "RicNewRftPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewRftPlotFeature::isCommandEnabled()
{
    if (RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot()) return false;

    //int branchIndex;

    RimSimWellInView* simWell = caf::firstAncestorOfTypeFromSelectedObject<RimSimWellInView*>();
    RimWellPath* rimWellPath = simWell == nullptr ? caf::firstAncestorOfTypeFromSelectedObject<RimWellPath*>() : nullptr;
    
    bool enable = true;
    if (simWell != nullptr)
    {
        RimEclipseResultCase* eclCase = caf::firstAncestorOfTypeFromSelectedObject<RimEclipseResultCase*>();
        if (simWell != nullptr)
        {
            enable &= RimWellRftPlot::hasPressureData(eclCase);
        }
    }
    else if (rimWellPath)
    {
        auto wellLogFile = rimWellPath->wellLogFile();
        if (wellLogFile != nullptr)
        {
            enable &= RimWellRftPlot::hasPressureData(wellLogFile);
        }
    }
    return enable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewRftPlotFeature::onActionTriggered(bool isChecked)
{
    //if (RicWellLogPlotCurveFeatureImpl::parentWellRftPlot()) return;

    RimProject* proj = RiaApplication::instance()->project();

    RimRftPlotCollection* rftPlotColl = proj->mainPlotCollection()->rftPlotCollection();
    if (rftPlotColl)
    {
        QString wellName;
        RimWellPath* wellPath = nullptr;
        RimSimWellInView* eclipseWell = nullptr;
        if ((wellPath = caf::firstAncestorOfTypeFromSelectedObject<RimWellPath*>()) != nullptr)
        {
            wellName = wellPath->name();
        }
        else if ((eclipseWell = caf::firstAncestorOfTypeFromSelectedObject<RimSimWellInView*>()) != nullptr)
        {
            wellName = eclipseWell->name();
        }

        QString plotName = QString(RimWellRftPlot::plotNameFormatString()).arg(wellName);

        RimWellRftPlot* rftPlot = new RimWellRftPlot();
        rftPlot->setCurrentWellName(wellName);

        RimWellLogTrack* plotTrack = new RimWellLogTrack();
        rftPlot->wellLogPlot()->addTrack(plotTrack);
        plotTrack->setDescription(QString("Track %1").arg(rftPlot->wellLogPlot()->trackCount()));

        rftPlotColl->addPlot(rftPlot);
        rftPlot->setDescription(plotName);

        rftPlot->applyInitialSelections();
        rftPlot->loadDataAndUpdate();
        rftPlotColl->updateConnectedEditors();

        RiuMainPlotWindow* mainPlotWindow = RiaApplication::instance()->getOrCreateAndShowMainPlotWindow();
        if (mainPlotWindow)
        {
            mainPlotWindow->setExpanded(plotTrack);
            mainPlotWindow->selectAsCurrentItem(rftPlot);
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
    auto selection = caf::selectedObjectsByType<RimWellLogTrack*>();
    return selection.size() > 0 ? selection[0] : nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath* RicNewRftPlotFeature::selectedWellPath() const
{
    auto selection = caf::selectedObjectsByType<RimWellPath*>();
    return selection.size() > 0 ? selection[0] : nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSimWellInView* RicNewRftPlotFeature::selectedSimulationWell(int * branchIndex) const
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
        std::vector<RimSimWellInView*> selection;
        caf::SelectionManager::instance()->objectsByType(&selection);
        (*branchIndex) = 0;
        return selection.size() > 0 ? selection[0] : nullptr;
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

