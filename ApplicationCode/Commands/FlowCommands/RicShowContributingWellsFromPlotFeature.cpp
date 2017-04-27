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

#include "RicShowContributingWellsFromPlotFeature.h"

#include "RiaApplication.h"

#include "RicShowContributingWellsFeatureImpl.h"

#include "RimEclipseResultCase.h"
#include "RimFlowDiagSolution.h"
#include "RimWellAllocationPlot.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicShowContributingWellsFromPlotFeature, "RicShowContributingWellsFromPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicShowContributingWellsFromPlotFeature::isCommandEnabled()
{
    RimWellAllocationPlot* wellAllocationPlot = dynamic_cast<RimWellAllocationPlot*>(RiaApplication::instance()->activePlotWindow());

    if (wellAllocationPlot) return true;
    
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowContributingWellsFromPlotFeature::onActionTriggered(bool isChecked)
{
    RimWellAllocationPlot* wellAllocationPlot = dynamic_cast<RimWellAllocationPlot*>(RiaApplication::instance()->activePlotWindow());

    if (!wellAllocationPlot) return;

    int timeStep = wellAllocationPlot->timeStep();
    QString wellName = wellAllocationPlot->wellName();

    RimEclipseResultCase* wellAllocationResultCase = wellAllocationPlot->rimCase();

    RicShowContributingWellsFeatureImpl::maniuplateSelectedView(wellAllocationResultCase, wellName, timeStep);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowContributingWellsFromPlotFeature::setupActionLook(QAction* actionToSetup)
{
    //actionToSetup->setIcon(QIcon(":/new_icon16x16.png"));
    actionToSetup->setText("Show Contributing Wells");
}
