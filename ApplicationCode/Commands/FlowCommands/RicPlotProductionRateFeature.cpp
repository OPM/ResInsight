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

#include "RicPlotProductionRateFeature.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RifEclipseSummaryAddress.h"

#include "RigSingleWellResultsData.h"

#include "RimEclipseResultCase.h"
#include "RimEclipseWell.h"
#include "RimGridSummaryCase.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveAppearanceCalculator.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RimView.h"

#include "RiuMainPlotWindow.h"
#include "RiuMainWindow.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicPlotProductionRateFeature, "RicPlotProductionRateFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicPlotProductionRateFeature::isCommandEnabled()
{
    std::vector<RimEclipseWell*> collection;
    caf::SelectionManager::instance()->objectsByType(&collection);

    for (RimEclipseWell* well : collection)
    {
        RimGridSummaryCase* gridSummaryCase = RicPlotProductionRateFeature::gridSummaryCaseForWell(well);
        if (gridSummaryCase)
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPlotProductionRateFeature::onActionTriggered(bool isChecked)
{
    RimProject* project = RiaApplication::instance()->project();
    CAF_ASSERT(project);

    RimSummaryCaseCollection* sumCaseColl = project->activeOilField() ? project->activeOilField()->summaryCaseCollection() : nullptr;
    if (!sumCaseColl) return;

    RimMainPlotCollection* mainPlotColl = project->mainPlotCollection();
    CAF_ASSERT(mainPlotColl);

    RimSummaryPlotCollection* summaryPlotColl = mainPlotColl->summaryPlotCollection();
    CAF_ASSERT(summaryPlotColl);

    std::vector<RimEclipseWell*> collection;
    caf::SelectionManager::instance()->objectsByType(&collection);

    RimSummaryPlot* summaryPlotToSelect = nullptr;

    for (RimEclipseWell* well : collection)
    {
        RimGridSummaryCase* gridSummaryCase = RicPlotProductionRateFeature::gridSummaryCaseForWell(well);
        if (!gridSummaryCase) continue;

        QString description = "Well Production Rates : ";

        if (isInjector(well))
        {
            description = "Well Injection Rates : ";
        }

        RimSummaryPlot* plot = new RimSummaryPlot();
        summaryPlotColl->summaryPlots().push_back(plot);

        description += well->name();
        plot->setDescription(description);

        if (isInjector(well))
        {
            // Left Axis

            RimDefines::PlotAxis plotAxis = RimDefines::PLOT_AXIS_LEFT;
            
            {
                // Water
                QString parameterName = "WWIR";
                RimSummaryCurve* curve = RicPlotProductionRateFeature::addSummaryCurve(plot, well, gridSummaryCase, parameterName, plotAxis);
                curve->setColor(RimSummaryCurveAppearanceCalculator::cycledBlueColor(0));
            }

            {
                // Gas
                QString parameterName = "WGIR";
                RimSummaryCurve* curve = RicPlotProductionRateFeature::addSummaryCurve(plot, well, gridSummaryCase, parameterName, plotAxis);
                curve->setColor(RimSummaryCurveAppearanceCalculator::cycledRedColor(0));
            }
        }
        else
        {
            // Left Axis

            RimDefines::PlotAxis plotAxis = RimDefines::PLOT_AXIS_LEFT;
            
            {
                // Oil
                QString parameterName = "WOPR";
                RimSummaryCurve* curve = RicPlotProductionRateFeature::addSummaryCurve(plot, well, gridSummaryCase, parameterName, plotAxis);
                curve->setColor(RimSummaryCurveAppearanceCalculator::cycledGreenColor(0));
            }

            {
                // Water
                QString parameterName = "WWPR";
                RimSummaryCurve* curve = RicPlotProductionRateFeature::addSummaryCurve(plot, well, gridSummaryCase, parameterName, plotAxis);
                curve->setColor(RimSummaryCurveAppearanceCalculator::cycledBlueColor(0));
            }

            {
                // Gas
                QString parameterName = "WGPR";
                RimSummaryCurve* curve = RicPlotProductionRateFeature::addSummaryCurve(plot, well, gridSummaryCase, parameterName, plotAxis);
                curve->setColor(RimSummaryCurveAppearanceCalculator::cycledRedColor(0));
            }
        }


        // Right Axis

        {
            RimDefines::PlotAxis plotAxis = RimDefines::PLOT_AXIS_RIGHT;

            {
                RimSummaryCurve* curve = RicPlotProductionRateFeature::addSummaryCurve(plot, well, gridSummaryCase, "WTHP", plotAxis);
                curve->setColor(RimSummaryCurveAppearanceCalculator::cycledNoneRGBBrColor(0));
                //curve->setSymbol(RimPlotCurve::SYMBOL_DIAMOND);
            }

            {
                RimSummaryCurve* curve = RicPlotProductionRateFeature::addSummaryCurve(plot, well, gridSummaryCase, "WBHP", plotAxis);
                curve->setColor(RimSummaryCurveAppearanceCalculator::cycledNoneRGBBrColor(1));
                //curve->setSymbol(RimPlotCurve::SYMBOL_DIAMOND);
            }
        
        }

        summaryPlotColl->updateConnectedEditors();
        plot->loadDataAndUpdate();

        summaryPlotToSelect = plot;
    }

    if (summaryPlotToSelect)
    {
        RiuMainPlotWindow* mainPlotWindow = RiaApplication::instance()->getOrCreateAndShowMainPlotWindow();
        if (mainPlotWindow)
        {
            mainPlotWindow->selectAsCurrentItem(summaryPlotToSelect);
            mainPlotWindow->setExpanded(summaryPlotToSelect, true);

            mainPlotWindow->tileWindows();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPlotProductionRateFeature::setupActionLook(QAction* actionToSetup)
{
    //actionToSetup->setIcon(QIcon(":/WellAllocPlot16x16.png"));
    actionToSetup->setText("Plot Production Rates");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGridSummaryCase* RicPlotProductionRateFeature::gridSummaryCaseForWell(RimEclipseWell* well)
{
    RimProject* project = RiaApplication::instance()->project();
    if (!project) return nullptr;

    RimSummaryCaseCollection* sumCaseColl = project->activeOilField() ? project->activeOilField()->summaryCaseCollection() : nullptr;
    if (!sumCaseColl) return nullptr;

    RimEclipseResultCase* eclCase = nullptr;
    well->firstAncestorOrThisOfType(eclCase);
    if (eclCase)
    {
        RimGridSummaryCase* gridSummaryCase = dynamic_cast<RimGridSummaryCase*>(sumCaseColl->findSummaryCaseFromEclipseResultCase(eclCase));
        if (gridSummaryCase)
        {
            return gridSummaryCase;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicPlotProductionRateFeature::isInjector(RimEclipseWell* well)
{
    RigSingleWellResultsData* wRes = well->wellResults();
    if (wRes)
    {
        RimView* rimView = nullptr;
        well->firstAncestorOrThisOfTypeAsserted(rimView);

        int currentTimeStep = rimView->currentTimeStep();

        if (wRes->hasWellResult(currentTimeStep))
        {
            const RigWellResultFrame& wrf = wRes->wellResultFrame(currentTimeStep);

            if (   wrf.m_productionType == RigWellResultFrame::OIL_INJECTOR
                || wrf.m_productionType == RigWellResultFrame::GAS_INJECTOR
                || wrf.m_productionType == RigWellResultFrame::WATER_INJECTOR)
            {
                return true;
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurve* RicPlotProductionRateFeature::addSummaryCurve( RimSummaryPlot* plot, const RimEclipseWell* well,
                                                    RimGridSummaryCase* gridSummaryCase, const QString& vectorName,
                                                    RimDefines::PlotAxis plotAxis)
{
    CVF_ASSERT(plot);
    CVF_ASSERT(gridSummaryCase);
    CVF_ASSERT(well);

    RimSummaryCurve* newCurve = new RimSummaryCurve();
    plot->addCurve(newCurve);

    newCurve->setSummaryCase(gridSummaryCase);

    RifEclipseSummaryAddress addr(RifEclipseSummaryAddress::SUMMARY_WELL,
        vectorName.toStdString(),
        -1,
        -1,
        "",
        well->name().toStdString(),
        -1,
        "",
        -1,
        -1,
        -1);

    newCurve->setSummaryAddress(addr);
    newCurve->setYAxis(plotAxis);

    return newCurve;
}

