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

#include "RicNewWellLogCurveExtractionFeature.h"

#include "RicWellLogPlotCurveFeatureImpl.h"
#include "RicNewWellLogPlotFeatureImpl.h"

#include "RiaApplication.h"

#include "RigWellLogCurveData.h"

#include "RimProject.h"
#include "RimView.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RiuMainPlotWindow.h"

#include "cafSelectionManager.h"

#include <QAction>

#include <vector>
#include "RimEclipseWell.h"
#include "RiuSelectionManager.h"


CAF_CMD_SOURCE_INIT(RicNewWellLogCurveExtractionFeature, "RicNewWellLogCurveExtractionFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellLogCurveExtractionFeature::isCommandEnabled()
{
    if (RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot()) return false;
    int branchIndex;
    return (selectedWellLogPlotTrack() != nullptr || selectedWellPath() != nullptr || selectedSimulationWell(&branchIndex) != nullptr) && caseAvailable();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellLogCurveExtractionFeature::onActionTriggered(bool isChecked)
{
    if (RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot()) return;

    RimWellLogTrack* wellLogPlotTrack = selectedWellLogPlotTrack();
    if (wellLogPlotTrack)
    {
        addCurve(wellLogPlotTrack, NULL, NULL, nullptr, -1);
    }
    else
    {
        RimWellPath* wellPath = selectedWellPath();
        int branchIndex = -1;
        RimEclipseWell* simWell = selectedSimulationWell(&branchIndex);
        if (wellPath || simWell)
        {
            RimWellLogTrack* wellLogPlotTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack();
            RimWellLogExtractionCurve* plotCurve = addCurve(wellLogPlotTrack, RiaApplication::instance()->activeReservoirView(), wellPath, simWell, branchIndex);

            plotCurve->loadDataAndUpdate();

            RimWellLogPlot* plot = NULL;
            wellLogPlotTrack->firstAncestorOrThisOfType(plot);
            if (plot && plotCurve->curveData())
            {
                plot->setDepthUnit(plotCurve->curveData()->depthUnit());
            }

            plotCurve->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellLogCurveExtractionFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Well Log Extraction Curve");
    actionToSetup->setIcon(QIcon(":/WellLogCurve16x16.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogTrack* RicNewWellLogCurveExtractionFeature::selectedWellLogPlotTrack() const
{
    std::vector<RimWellLogTrack*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    return selection.size() > 0 ? selection[0] : NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath* RicNewWellLogCurveExtractionFeature::selectedWellPath() const
{
    std::vector<RimWellPath*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    return selection.size() > 0 ? selection[0] : NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseWell* RicNewWellLogCurveExtractionFeature::selectedSimulationWell(int * branchIndex) const
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
bool RicNewWellLogCurveExtractionFeature::caseAvailable() const
{
    std::vector<RimCase*> cases;
    RiaApplication::instance()->project()->allCases(cases);

    return cases.size() > 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogExtractionCurve* RicNewWellLogCurveExtractionFeature::addCurve(RimWellLogTrack* plotTrack, RimView* view, RimWellPath* wellPath, const RimEclipseWell* simWell, int branchIndex)
{
    CVF_ASSERT(plotTrack);
    RimWellLogExtractionCurve* curve = new RimWellLogExtractionCurve();

    cvf::Color3f curveColor = RicWellLogPlotCurveFeatureImpl::curveColorFromTable(plotTrack->curveCount());
    curve->setColor(curveColor);
    if (wellPath) curve->setWellPath(wellPath);
    if (simWell) curve->setFromSimulationWellName(simWell->name(), branchIndex);

    curve->setPropertiesFromView(view);

    plotTrack->addCurve(curve);

    plotTrack->updateConnectedEditors();

    // Make sure the summary plot window is created and visible
    RiuMainPlotWindow* plotwindow = RiaApplication::instance()->getOrCreateAndShowMainPlotWindow();

    RiaApplication::instance()->project()->updateConnectedEditors();

    plotwindow->selectAsCurrentItem(curve);

    return curve;
}
