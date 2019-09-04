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

#include "RicNewWellLogPlotFeatureImpl.h"
#include "RicWellLogPlotCurveFeatureImpl.h"
#include "RicWellLogTools.h"

#include "RiaApplication.h"

#include "RigWellLogCurveData.h"

#include "Rim3dView.h"
#include "RimProject.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "Riu3dSelectionManager.h"
#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

#include <vector>

CAF_CMD_SOURCE_INIT(RicNewWellLogCurveExtractionFeature, "RicNewWellLogCurveExtractionFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewWellLogCurveExtractionFeature::isCommandEnabled()
{
    if (RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot()) return false;
    if (RicWellLogPlotCurveFeatureImpl::parentWellRftPlot()) return false;
    int branchIndex;
    return (caf::SelectionManager::instance()->selectedItemOfType<RimWellLogTrack>() != nullptr ||
            caf::SelectionManager::instance()->selectedItemOfType<RimWellPath>() != nullptr ||
            RicWellLogTools::selectedSimulationWell(&branchIndex) != nullptr) &&
           caseAvailable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellLogCurveExtractionFeature::onActionTriggered(bool isChecked)
{
    if (RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot()) return;

    RimWellLogTrack* wellLogPlotTrack = caf::SelectionManager::instance()->selectedItemOfType<RimWellLogTrack>();
    if (wellLogPlotTrack)
    {
        RicWellLogTools::addExtractionCurve(wellLogPlotTrack, nullptr, nullptr, nullptr, -1, true);
    }
    else
    {
        RimWellPath*      wellPath    = caf::SelectionManager::instance()->selectedItemOfType<RimWellPath>();
        int               branchIndex = -1;
        RimSimWellInView* simWell     = RicWellLogTools::selectedSimulationWell(&branchIndex);

        bool                        useBranchDetection = true;
        RimSimWellInViewCollection* simWellColl        = nullptr;
        if (simWell)
        {
            simWell->firstAncestorOrThisOfTypeAsserted(simWellColl);
            useBranchDetection = simWellColl->isAutoDetectingBranches;
        }

        if (wellPath || simWell)
        {
            RimWellLogTrack* newWellLogPlotTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack();

            RimWellLogExtractionCurve* plotCurve =
                RicWellLogTools::addExtractionCurve(newWellLogPlotTrack,
                                                    RiaApplication::instance()->activeReservoirView(),
                                                    wellPath,
                                                    simWell,
                                                    branchIndex,
                                                    useBranchDetection);

            plotCurve->loadDataAndUpdate(true);

            RimWellLogPlot* plot = nullptr;
            newWellLogPlotTrack->firstAncestorOrThisOfType(plot);
            if (plot && plotCurve->curveData())
            {
                plot->setDepthUnit(plotCurve->curveData()->depthUnit());
            }

            plotCurve->updateConnectedEditors();
        }
    }
    RiuPlotMainWindowTools::refreshToolbars();
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
bool RicNewWellLogCurveExtractionFeature::caseAvailable()
{
    std::vector<RimCase*> cases;
    RiaApplication::instance()->project()->allCases(cases);

    return !cases.empty();
}
