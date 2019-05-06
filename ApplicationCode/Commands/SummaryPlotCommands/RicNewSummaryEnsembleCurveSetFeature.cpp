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

#include "RicNewSummaryEnsembleCurveSetFeature.h"

#include "RiaGuiApplication.h"
#include "RiaColorTables.h"

#include "RiaSummaryTools.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimEnsembleCurveSetColorManager.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "RiuPlotMainWindow.h"

#include "WellLogCommands/RicWellLogPlotCurveFeatureImpl.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicNewSummaryEnsembleCurveSetFeature, "RicNewSummaryEnsembleCurveSetFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet* RicNewSummaryEnsembleCurveSetFeature::addDefaultCurveSet(RimSummaryPlot*           plot,
                                                                              RimSummaryCaseCollection* ensemble)
{
    CVF_ASSERT(plot && ensemble);

    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

    RimEnsembleCurveSet* curveSet = new RimEnsembleCurveSet();

    // Use same counting as RicNewSummaryCurveFeature::onActionTriggered
    auto colorIndex = plot->singleColorCurveCount();
    curveSet->setColor(RiaColorTables::summaryCurveDefaultPaletteColors().cycledColor3f(colorIndex));
    curveSet->legendConfig()->setColorRange(
        RimEnsembleCurveSetColorManager::cycledEnsembleColorRange(static_cast<int>(colorIndex)));

    curveSet->setSummaryCaseCollection(ensemble);
    curveSet->setSummaryAddress(RifEclipseSummaryAddress::fieldAddress("FOPT"));

    plot->ensembleCurveSetCollection()->addCurveSet(curveSet);

    return curveSet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryEnsembleCurveSetFeature::createPlotForCurveSetAndUpdate(RimSummaryCaseCollection* ensemble)
{
    RiaGuiApplication* app  = RiaGuiApplication::instance();
    RimProject*     proj = app->project();

    RimSummaryPlotCollection* summaryPlotCollection = proj->mainPlotCollection->summaryPlotCollection();
    RimSummaryPlot*           plot                  = summaryPlotCollection->createSummaryPlotWithAutoTitle();

    RimEnsembleCurveSet* curveSet = RicNewSummaryEnsembleCurveSetFeature::addDefaultCurveSet(plot, ensemble);
    plot->loadDataAndUpdate();
    summaryPlotCollection->updateConnectedEditors();

    RiuPlotMainWindow* mainPlotWindow = app->getOrCreateAndShowMainPlotWindow();
    if (mainPlotWindow)
    {
        mainPlotWindow->selectAsCurrentItem(curveSet);
        mainPlotWindow->updateSummaryPlotToolBar();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewSummaryEnsembleCurveSetFeature::isCommandEnabled()
{
    return (selectedSummaryPlot());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryEnsembleCurveSetFeature::onActionTriggered(bool isChecked)
{
    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

    RimSummaryPlot* plot = selectedSummaryPlot();
    if (plot)
    {
        auto ensemble = project->summaryGroups().back();

        RicNewSummaryEnsembleCurveSetFeature::createPlotForCurveSetAndUpdate(ensemble);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryEnsembleCurveSetFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Ensemble Curve Set");
    actionToSetup->setIcon(QIcon(":/EnsembleCurveSet16x16.png"));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicNewSummaryEnsembleCurveSetFeature::selectedSummaryPlot() const
{
    RimSummaryPlot* sumPlot = nullptr;

    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>(caf::SelectionManager::instance()->selectedItem());
    if (selObj)
    {
        sumPlot = RiaSummaryTools::parentSummaryPlot(selObj);
    }

    return sumPlot;
}
