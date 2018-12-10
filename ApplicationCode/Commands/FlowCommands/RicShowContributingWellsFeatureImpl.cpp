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

#include "RicShowContributingWellsFeatureImpl.h"

#include "RiaApplication.h"

#include "RicSelectOrCreateViewFeatureImpl.h"

#include "RigFlowDiagResultAddress.h"
#include "RigSimWellData.h"

#include "RimEclipseCellColors.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimSimWellInViewCollection.h"
#include "RimFaultInViewCollection.h"
#include "RimFlowDiagSolution.h"
#include "RimProject.h"
#include "RimSimWellInView.h"
#include "RimViewManipulator.h"

#include "Riu3DMainWindowTools.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseView* RicShowContributingWellsFeatureImpl::maniuplateSelectedView(RimEclipseResultCase* eclipseResultCase, QString wellName, int timeStep)
{
    RimEclipseView* viewToManipulate = RicSelectOrCreateViewFeatureImpl::showViewSelection(eclipseResultCase, "lastUsedWellAllocationView", "Show Contributing Wells in View");

   if (!viewToManipulate) return nullptr;


    RicShowContributingWellsFeatureImpl::modifyViewToShowContributingWells(viewToManipulate, wellName, timeStep);

    auto* feature = caf::CmdFeatureManager::instance()->getCommandFeature("RicShowMainWindowFeature");
    feature->actionTriggered(false);

    RicSelectOrCreateViewFeatureImpl::focusView(viewToManipulate);

    return viewToManipulate;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowContributingWellsFeatureImpl::modifyViewToShowContributingWells(RimEclipseView* viewToModify, const QString& wellName, int timeStep)
{
    CVF_ASSERT(viewToModify);

    RimSimWellInView* selectedWell = nullptr;

    for (RimSimWellInView* w : viewToModify->wellCollection()->wells())
    {
        if (w->name() == wellName)
        {
            selectedWell = w;
        }
    }

    CVF_ASSERT(selectedWell);

    RimEclipseResultCase* eclipseResultCase = nullptr;
    selectedWell->firstAncestorOrThisOfTypeAsserted(eclipseResultCase);

    // Use the active flow diag solutions, or the first one as default
    RimFlowDiagSolution* flowDiagSolution = viewToModify->cellResult()->flowDiagSolution();
    if (!flowDiagSolution)
    {
        flowDiagSolution = eclipseResultCase->defaultFlowDiagSolution();
    }

    //assert(flowDiagSolution);
    CVF_ASSERT(flowDiagSolution);

    RimFlowDiagSolution::TracerStatusType tracerStatus = flowDiagSolution->tracerStatusInTimeStep(selectedWell->name(), timeStep);
    if (!(tracerStatus == RimFlowDiagSolution::INJECTOR || tracerStatus == RimFlowDiagSolution::PRODUCER))
    {
        return;
    }
    
    viewToModify->setCurrentTimeStep(timeStep);
    viewToModify->cellResult()->setResultType(RiaDefines::FLOW_DIAGNOSTICS);
    viewToModify->cellResult()->setResultVariable("MaxFractionTracer");
    viewToModify->cellResult()->setFlowSolution(flowDiagSolution);

    switch (tracerStatus)
    {
    case RimFlowDiagSolution::PRODUCER:
        viewToModify->cellResult()->setFlowDiagTracerSelectionType(RimEclipseResultDefinition::FLOW_TR_INJECTORS);
        break;
    case RimFlowDiagSolution::INJECTOR:
        viewToModify->cellResult()->setFlowDiagTracerSelectionType(RimEclipseResultDefinition::FLOW_TR_PRODUCERS);
        break;

    default:
        CVF_ASSERT(false);
        break;
    }

    viewToModify->cellResult()->loadDataAndUpdate();
    viewToModify->cellResult()->updateConnectedEditors();
    
    std::vector<QString> tracerNames = findContributingTracerNames(flowDiagSolution, selectedWell->simWellData(), timeStep);

    for (RimSimWellInView* w : viewToModify->wellCollection()->wells())
    {
        if (std::find(tracerNames.begin(), tracerNames.end(), w->name()) != tracerNames.end()
            || selectedWell->name() == w->name())
        {
            w->showWell = true;
        }
        else
        {
            w->showWell = false;
        }
    }

    // Disable all existing property filters, and
    // create a new property filter based on TOF for current well

    RimEclipsePropertyFilterCollection* propertyFilterCollection = viewToModify->eclipsePropertyFilterCollection();

    for (RimEclipsePropertyFilter* f : propertyFilterCollection->propertyFilters())
    {
        f->isActive = false;
    }

    RimEclipsePropertyFilter* propertyFilter = new RimEclipsePropertyFilter();
    propertyFilterCollection->propertyFilters().push_back(propertyFilter);

    propertyFilter->resultDefinition()->setEclipseCase(viewToModify->eclipseCase());
    propertyFilter->resultDefinition()->setTofAndSelectTracer(selectedWell->name());
    propertyFilter->resultDefinition()->loadDataAndUpdate();

    propertyFilterCollection->updateConnectedEditors();

    Riu3DMainWindowTools::setExpanded(propertyFilterCollection);

    viewToModify->faultCollection()->showFaultCollection = false;
    viewToModify->faultCollection()->updateConnectedEditors();

    viewToModify->updateCurrentTimeStepAndRedraw();
    viewToModify->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QString> RicShowContributingWellsFeatureImpl::findContributingTracerNames(
    const RimFlowDiagSolution* flowDiagSolution,
    const RigSimWellData* simWellData,
    int timeStep)
{
    std::vector<QString> tracerCellFractionValues;

    if (flowDiagSolution && simWellData->hasWellResult(timeStep))
    {
        RimFlowDiagSolution::TracerStatusType requestedTracerType = RimFlowDiagSolution::UNDEFINED;

        const RigWellResultFrame::WellProductionType prodType = simWellData->wellProductionType(timeStep);
        if (   prodType == RigWellResultFrame::PRODUCER
            || prodType == RigWellResultFrame::UNDEFINED_PRODUCTION_TYPE)
        {
            requestedTracerType = RimFlowDiagSolution::INJECTOR;
        }
        else
        {
            requestedTracerType = RimFlowDiagSolution::PRODUCER;
        }

        std::vector<QString> tracerNames = flowDiagSolution->tracerNames();
        for (const QString& tracerName : tracerNames)
        {
            if (flowDiagSolution->tracerStatusInTimeStep(tracerName, timeStep) == requestedTracerType)
            {
                tracerCellFractionValues.push_back(tracerName);
            }
        }
    }

    return tracerCellFractionValues;
}

