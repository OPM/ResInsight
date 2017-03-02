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

#include "RimDefines.h"
#include "RimEclipseCellColors.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimEclipseWellCollection.h"
#include "RimWellAllocationPlot.h"

#include "RiuMainWindow.h"

#include <QAction>
#include "cafCmdFeatureManager.h"
#include "RimFlowDiagSolution.h"

CAF_CMD_SOURCE_INIT(RicShowContributingWellsFromPlotFeature, "RicShowContributingWellsFromPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicShowContributingWellsFromPlotFeature::isCommandEnabled()
{
    RimWellAllocationPlot* wellAllocationPlot = RiaApplication::instance()->activeWellAllocationPlot();
    if (!wellAllocationPlot) return false;

    RimEclipseView* activeView = dynamic_cast<RimEclipseView*>(RiaApplication::instance()->activeReservoirView());
    if (!activeView) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowContributingWellsFromPlotFeature::onActionTriggered(bool isChecked)
{
    RimWellAllocationPlot* wellAllocationPlot = RiaApplication::instance()->activeWellAllocationPlot();
    if (!wellAllocationPlot) return;

    RimEclipseView* activeView = dynamic_cast<RimEclipseView*>(RiaApplication::instance()->activeReservoirView());
    if (!activeView) return;

    int timeStep = wellAllocationPlot->timeStep();
    QString wellName = wellAllocationPlot->wellName();
    const std::vector<QString> contributingTracers = wellAllocationPlot->contributingTracerNames();
    RimFlowDiagSolution* flowSolution = wellAllocationPlot->flowDiagSolution();

    if ( !flowSolution ) return;

    RimFlowDiagSolution::TracerStatusType tracerStatus = flowSolution->tracerStatusInTimeStep(wellName, timeStep);
    
    if (!(tracerStatus == RimFlowDiagSolution::INJECTOR || tracerStatus == RimFlowDiagSolution::PRODUCER) ) return;

    activeView->cellResult()->setResultType(RimDefines::FLOW_DIAGNOSTICS);
    activeView->cellResult()->setResultVariable("MaxFractionTracer");

    switch (tracerStatus)
    {
        case RimFlowDiagSolution::PRODUCER:
        activeView->cellResult()->setFlowDiagTracerSelectionType(RimEclipseResultDefinition::FLOW_TR_INJECTORS);
        break;
        case RimFlowDiagSolution::INJECTOR:
        activeView->cellResult()->setFlowDiagTracerSelectionType(RimEclipseResultDefinition::FLOW_TR_PRODUCERS);
        break;

        default:
        CVF_ASSERT(false);
        break;
    }
    activeView->setCurrentTimeStep(timeStep);
    activeView->cellResult()->loadDataAndUpdate();

    activeView->cellResult()->updateConnectedEditors();


    for ( RimEclipseWell* well : activeView->wellCollection()->wells() )
    {
        if ( std::find(contributingTracers.begin(), contributingTracers.end(), well->name()) != contributingTracers.end()
            || wellAllocationPlot->wellName() == well->name() )
        {
            well->showWell = true;
        }
        else
        {
            well->showWell = false;
        }
    }

    // Disable all existing property filters, and
    // create a new property filter based on TOF for current well

    RimEclipsePropertyFilterCollection* propertyFilterCollection = activeView->eclipsePropertyFilterCollection();

    for ( RimEclipsePropertyFilter* f : propertyFilterCollection->propertyFilters() )
    {
        f->isActive = false;
    }

    RimEclipsePropertyFilter* propertyFilter = new RimEclipsePropertyFilter();
    propertyFilterCollection->propertyFilters().push_back(propertyFilter);

    propertyFilter->resultDefinition()->setEclipseCase(activeView->eclipseCase());
    propertyFilter->resultDefinition()->setTofAndSelectTracer(wellAllocationPlot->wellName());
    propertyFilter->resultDefinition()->loadDataAndUpdate();

    propertyFilterCollection->updateConnectedEditors();

    RiuMainWindow::instance()->setExpanded(propertyFilterCollection, true);

    activeView->scheduleCreateDisplayModelAndRedraw();

    auto* feature = caf::CmdFeatureManager::instance()->getCommandFeature("RicShowMainWindowFeature");
    feature->actionTriggered(false);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowContributingWellsFromPlotFeature::setupActionLook(QAction* actionToSetup)
{
    //actionToSetup->setIcon(QIcon(":/new_icon16x16.png"));
    actionToSetup->setText("Show Contributing Wells");
}
