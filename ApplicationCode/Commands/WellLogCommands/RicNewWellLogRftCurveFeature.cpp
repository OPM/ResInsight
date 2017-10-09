/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017  Statoil ASA
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

#include "RicNewWellLogRftCurveFeature.h"

#include "RiaApplication.h"

#include "RimEclipseWell.h"
#include "RimProject.h"
#include "RimWellLogRftCurve.h"
#include "RimWellLogTrack.h"
#include "RimWellLogPlot.h"
#include "RimWellLogCurve.h"
#include "RimEclipseResultCase.h"

#include "RigWellLogCurveData.h"

#include "RifReaderEclipseRft.h"

#include "RiuMainPlotWindow.h"
#include "RiuSelectionManager.h"

#include "RicWellLogPlotCurveFeatureImpl.h"
#include "RicNewWellLogPlotFeatureImpl.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QString>

#include <vector>



CAF_CMD_SOURCE_INIT(RicNewWellLogRftCurveFeature, "RicNewWellLogRftCurveFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellLogRftCurveFeature::isCommandEnabled()
{

    if (RicNewWellLogRftCurveFeature::selectedWellLogPlotTrack() != nullptr)
    {
        return true;
    }

    int branchIdx;
    RimEclipseWell* simulationWell = RicNewWellLogRftCurveFeature::selectedSimulationWell(&branchIdx);

    if (simulationWell != nullptr)
    {
        return RicNewWellLogRftCurveFeature::wellHasRftData(simulationWell->name());
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellLogRftCurveFeature::onActionTriggered(bool isChecked)
{
    RimWellLogTrack* wellLogPlotTrack = selectedWellLogPlotTrack();
    if (wellLogPlotTrack)
    {
        int branchIdx;
        RicNewWellLogRftCurveFeature::addCurve(wellLogPlotTrack, selectedSimulationWell(&branchIdx));
    }
    else
    {
        int branchIndex = -1;
        RimEclipseWell* simWell = selectedSimulationWell(&branchIndex);
        if (simWell)
        {
            RimWellLogTrack* wellLogPlotTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack();
            RimWellLogRftCurve* plotCurve = RicNewWellLogRftCurveFeature::addCurve(wellLogPlotTrack, simWell);

            plotCurve->loadDataAndUpdate(true);

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
void RicNewWellLogRftCurveFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Well Log RFT Curve");
    actionToSetup->setIcon(QIcon(":/WellLogCurve16x16.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogRftCurve* RicNewWellLogRftCurveFeature::addCurve(RimWellLogTrack* plotTrack, const RimEclipseWell* simWell)
{
    CVF_ASSERT(plotTrack);

    RimWellLogRftCurve* curve = new RimWellLogRftCurve();

    RimEclipseResultCase* resultCase;
    
    std::vector<RimCase*> cases;
    RiaApplication::instance()->project()->allCases(cases);

    for (RimCase* rimCase : cases)
    {
        if (resultCase = dynamic_cast<RimEclipseResultCase*>(rimCase))
        {
            break;
        }
    }

    if (simWell && resultCase)
    {
        curve->setEclipseResultCase(resultCase);
        curve->setDefaultAddress(simWell->name());
    }

    cvf::Color3f curveColor = RicWellLogPlotCurveFeatureImpl::curveColorFromTable(plotTrack->curveCount());
    curve->setColor(curveColor);

    plotTrack->addCurve(curve);
    plotTrack->updateConnectedEditors();

    RiuMainPlotWindow* plotwindow = RiaApplication::instance()->getOrCreateAndShowMainPlotWindow();

    RiaApplication::instance()->project()->updateConnectedEditors();

    plotwindow->selectAsCurrentItem(curve);
    
    return curve;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogTrack* RicNewWellLogRftCurveFeature::selectedWellLogPlotTrack()
{
    std::vector<RimWellLogTrack*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    return selection.size() > 0 ? selection[0] : nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseWell* RicNewWellLogRftCurveFeature::selectedSimulationWell(int *branchIndex) 
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
bool RicNewWellLogRftCurveFeature::wellHasRftData(const QString& wellName)
{
    RimEclipseResultCase* resultCase;
    std::vector<RimCase*> cases;
    RiaApplication::instance()->project()->allCases(cases);

    for (RimCase* rimCase : cases)
    {
        if (resultCase = dynamic_cast<RimEclipseResultCase*>(rimCase))
        {
            return resultCase->rftReader()->wellHasRftData(wellName);
        }
    }

    return false;
}
