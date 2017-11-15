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
#include "RifReaderEclipseSummary.h"

#include "RigSimWellData.h"

#include "RimEclipseResultCase.h"
#include "RimGridSummaryCase.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSimWellInView.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveAppearanceCalculator.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RimView.h"
#include "RiaSummaryTools.h"

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
    std::vector<RimSimWellInView*> collection;
    caf::SelectionManager::instance()->objectsByType(&collection);

    for (RimSimWellInView* well : collection)
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

    RimSummaryCaseMainCollection* sumCaseColl = project->activeOilField() ? project->activeOilField()->summaryCaseMainCollection() : nullptr;
    if (!sumCaseColl) return;

    RimSummaryPlotCollection* summaryPlotColl = RiaSummaryTools::summaryPlotCollection();

    std::vector<RimSimWellInView*> collection;
    caf::SelectionManager::instance()->objectsByType(&collection);

    RimSummaryPlot* summaryPlotToSelect = nullptr;

    for (RimSimWellInView* well : collection)
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

            RiaDefines::PlotAxis plotAxis = RiaDefines::PLOT_AXIS_LEFT;
            
            {
                // Note : The parameter "WOIR" is probably never-existing, but we check for existence before creating curve
                // Oil
                QString parameterName = "WOIR";
                RicPlotProductionRateFeature::addSummaryCurve(plot, well, gridSummaryCase, parameterName,
                    plotAxis, RimSummaryCurveAppearanceCalculator::cycledGreenColor(0));
            }

            {
                // Water
                QString parameterName = "WWIR";
                RicPlotProductionRateFeature::addSummaryCurve(plot, well, gridSummaryCase, parameterName,
                    plotAxis, RimSummaryCurveAppearanceCalculator::cycledBlueColor(0));
            }

            {
                // Gas
                QString parameterName = "WGIR";
                RicPlotProductionRateFeature::addSummaryCurve(plot, well, gridSummaryCase, parameterName,
                    plotAxis, RimSummaryCurveAppearanceCalculator::cycledRedColor(0));
            }
        }
        else
        {
            // Left Axis

            RiaDefines::PlotAxis plotAxis = RiaDefines::PLOT_AXIS_LEFT;
            
            {
                // Oil
                QString parameterName = "WOPR";
                RicPlotProductionRateFeature::addSummaryCurve(plot, well, gridSummaryCase, parameterName,
                    plotAxis, RimSummaryCurveAppearanceCalculator::cycledGreenColor(0));
            }

            {
                // Water
                QString parameterName = "WWPR";
                RicPlotProductionRateFeature::addSummaryCurve(plot, well, gridSummaryCase, parameterName,
                    plotAxis, RimSummaryCurveAppearanceCalculator::cycledBlueColor(0));
            }

            {
                // Gas
                QString parameterName = "WGPR";
                RicPlotProductionRateFeature::addSummaryCurve(plot, well, gridSummaryCase, parameterName,
                    plotAxis, RimSummaryCurveAppearanceCalculator::cycledRedColor(0));
            }
        }


        // Right Axis

        {
            RiaDefines::PlotAxis plotAxis = RiaDefines::PLOT_AXIS_RIGHT;

            {
                QString parameterName = "WTHP";
                RicPlotProductionRateFeature::addSummaryCurve(plot, well, gridSummaryCase, parameterName,
                    plotAxis, RimSummaryCurveAppearanceCalculator::cycledNoneRGBBrColor(0));
            }

            {
                QString parameterName = "WBHP";
                RicPlotProductionRateFeature::addSummaryCurve(plot, well, gridSummaryCase, parameterName,
                    plotAxis, RimSummaryCurveAppearanceCalculator::cycledNoneRGBBrColor(1));
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
            mainPlotWindow->setExpanded(summaryPlotToSelect);

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
RimGridSummaryCase* RicPlotProductionRateFeature::gridSummaryCaseForWell(RimSimWellInView* well)
{
    RimProject* project = RiaApplication::instance()->project();
    if (!project) return nullptr;

    RimSummaryCaseMainCollection* sumCaseColl = project->activeOilField() ? project->activeOilField()->summaryCaseMainCollection() : nullptr;
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
bool RicPlotProductionRateFeature::isInjector(RimSimWellInView* well)
{
    RigSimWellData* wRes = well->simWellData();
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
RimSummaryCurve* RicPlotProductionRateFeature::addSummaryCurve( RimSummaryPlot* plot, const RimSimWellInView* well,
                                                    RimGridSummaryCase* gridSummaryCase, const QString& vectorName,
                                                    RiaDefines::PlotAxis plotAxis, const cvf::Color3f& color)
{
    CVF_ASSERT(plot);
    CVF_ASSERT(gridSummaryCase);
    CVF_ASSERT(well);

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

    if (!gridSummaryCase->summaryReader()->hasAddress(addr))
    {
        return nullptr;
    }

    RimSummaryCurve* newCurve = new RimSummaryCurve();
    plot->addCurveAndUpdate(newCurve);

    newCurve->setSummaryCaseY(gridSummaryCase);
    newCurve->setSummaryAddressY(addr);
    newCurve->setColor(color);
    newCurve->setLeftOrRightAxisY(plotAxis);

    return newCurve;
}

