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

#include "RimEclipseResultCase.h"
#include "RimProject.h"
#include "RimSimWellInView.h"
#include "RimWellLogCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogRftCurve.h"
#include "RimWellLogTrack.h"

#include "RigWellLogCurveData.h"

#include "RiuMainPlotWindow.h"

#include "RicNewWellLogPlotFeatureImpl.h"
#include "RicWellLogPlotCurveFeatureImpl.h"
#include "RicWellLogTools.h"

#include <QAction>
#include <QString>

#include <vector>


CAF_CMD_SOURCE_INIT(RicNewWellLogRftCurveFeature, "RicNewWellLogRftCurveFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellLogRftCurveFeature::isCommandEnabled()
{
    if (RicWellLogPlotCurveFeatureImpl::parentWellRftPlot()) return false;
    if (RicWellLogTools::selectedWellLogPlotTrack() != nullptr)
    {
        return true;
    }

    int branchIdx;
    RimSimWellInView* simulationWell = RicWellLogTools::selectedSimulationWell(&branchIdx);

    if (simulationWell != nullptr)
    {
        return RicWellLogTools::wellHasRftData(simulationWell->name());
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellLogRftCurveFeature::onActionTriggered(bool isChecked)
{
    RimWellLogTrack* wellLogPlotTrack = RicWellLogTools::selectedWellLogPlotTrack();
    if (wellLogPlotTrack)
    {
        int branchIdx;
        RicWellLogTools::addRftCurve(wellLogPlotTrack, RicWellLogTools::selectedSimulationWell(&branchIdx));
    }
    else
    {
        int branchIndex = -1;
        RimSimWellInView* simWell = RicWellLogTools::selectedSimulationWell(&branchIndex);
        if (simWell)
        {
            RimWellLogTrack* wellLogPlotTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack();
            RimWellLogRftCurve* plotCurve = RicWellLogTools::addRftCurve(wellLogPlotTrack, simWell);

            plotCurve->loadDataAndUpdate(true);

            RimWellLogPlot* plot = nullptr;
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
