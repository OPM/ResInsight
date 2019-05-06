/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicNewSummaryCurveFeature.h"

#include "RiaGuiApplication.h"
#include "RiaColorTables.h"

#include "RiaSummaryTools.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "RiuPlotMainWindow.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>
#include "RiuPlotMainWindowTools.h"

CAF_CMD_SOURCE_INIT(RicNewSummaryCurveFeature, "RicNewSummaryCurveFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurve* RicNewSummaryCurveFeature::addCurveToPlot(RimSummaryPlot* plot, RimSummaryCase* summaryCase)
{
    if (plot)
    {
        RimSummaryCurve* newCurve = new RimSummaryCurve();

        // Use same counting as RicNewSummaryEnsembleCurveSetFeature::onActionTriggered
        cvf::Color3f curveColor = RiaColorTables::summaryCurveDefaultPaletteColors().cycledColor3f(plot->singleColorCurveCount());
        newCurve->setColor(curveColor);

        plot->addCurveAndUpdate(newCurve);

        if (summaryCase)
        {
            newCurve->setSummaryCaseY(summaryCase);
        }

        newCurve->setSummaryAddressYAndApplyInterpolation(RifEclipseSummaryAddress::fieldAddress("FOPT"));

        newCurve->loadDataAndUpdate(true);

        return newCurve;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSummaryCurveFeature::ensureAtLeastOnePlot(RimSummaryPlotCollection* summaryPlotCollection, RimSummaryCase* summaryCase)
{
    if (summaryPlotCollection && summaryCase)
    {
        if (summaryPlotCollection->summaryPlots.empty())
        {
            createNewPlot(summaryPlotCollection, summaryCase);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryCurveFeature::createNewPlot(RimSummaryPlotCollection* summaryPlotCollection, RimSummaryCase* summaryCase)
{
    if (summaryPlotCollection && summaryCase)
    {
        auto plot = summaryPlotCollection->createSummaryPlotWithAutoTitle();

        auto curve = RicNewSummaryCurveFeature::addCurveToPlot(plot, summaryCase);
        plot->loadDataAndUpdate();

        summaryPlotCollection->updateConnectedEditors();

        RiuPlotMainWindowTools::setExpanded(curve);
        RiuPlotMainWindowTools::selectAsCurrentItem(curve);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewSummaryCurveFeature::isCommandEnabled()
{
    return (selectedSummaryPlot());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryCurveFeature::onActionTriggered(bool isChecked)
{
    RiaGuiApplication* app = RiaGuiApplication::instance();
    RimProject* project = app->project();
    CVF_ASSERT(project);

    RimSummaryPlot* plot = selectedSummaryPlot();
    if (plot)
    {
        RimSummaryCase* defaultCase = nullptr;
        if (project->activeOilField()->summaryCaseMainCollection()->summaryCaseCount() > 0)
        {
            defaultCase = project->activeOilField()->summaryCaseMainCollection()->summaryCase(0);
        }

        RimSummaryCurve* newCurve = addCurveToPlot(plot, defaultCase);

        plot->updateConnectedEditors();

        app->getOrCreateAndShowMainPlotWindow()->selectAsCurrentItem(newCurve);

        RiuPlotMainWindow* mainPlotWindow = app->mainPlotWindow();
        mainPlotWindow->updateSummaryPlotToolBar();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryCurveFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Summary Curve");
    actionToSetup->setIcon(QIcon(":/SummaryCurve16x16.png"));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicNewSummaryCurveFeature::selectedSummaryPlot() const
{
    RimSummaryPlot* sumPlot = nullptr;

    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>(caf::SelectionManager::instance()->selectedItem());
    if (selObj)
    {
        sumPlot = RiaSummaryTools::parentSummaryPlot(selObj);
    }

    return sumPlot;
}
