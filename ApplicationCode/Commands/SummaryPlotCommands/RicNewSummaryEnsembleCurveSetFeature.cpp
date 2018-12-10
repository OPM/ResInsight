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

#include "RiaApplication.h"
#include "RiaColorTables.h"

#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RiaSummaryTools.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimEnsembleCurveSetColorManager.h"

#include "RiuPlotMainWindow.h"

#include "WellLogCommands/RicWellLogPlotCurveFeatureImpl.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicNewSummaryEnsembleCurveSetFeature, "RicNewSummaryEnsembleCurveSetFeature");

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
        RimEnsembleCurveSet* curveSet = new RimEnsembleCurveSet();

        // Set single curve set color
        auto allCurveSets = plot->ensembleCurveSetCollection()->curveSets();
        size_t colorIndex = std::count_if(allCurveSets.begin(), allCurveSets.end(), [](RimEnsembleCurveSet* curveSet)
        {
            return curveSet->colorMode() == RimEnsembleCurveSet::SINGLE_COLOR;
        });
        curveSet->setColor(RiaColorTables::summaryCurveDefaultPaletteColors().cycledColor3f(colorIndex));
        curveSet->legendConfig()->setColorRange(RimEnsembleCurveSetColorManager::cycledEnsembleColorRange(static_cast<int>(colorIndex)));

        if (!project->summaryGroups().empty())
        {
            curveSet->setSummaryCaseCollection(project->summaryGroups().back());
        }

        plot->ensembleCurveSetCollection()->addCurveSet(curveSet);
        plot->updateConnectedEditors();

        RiaApplication::instance()->getOrCreateAndShowMainPlotWindow()->selectAsCurrentItem(curveSet);

        RiuPlotMainWindow* mainPlotWindow = RiaApplication::instance()->mainPlotWindow();
        mainPlotWindow->updateSummaryPlotToolBar();
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

    caf::PdmObject* selObj =  dynamic_cast<caf::PdmObject*>(caf::SelectionManager::instance()->selectedItem());
    if (selObj)
    {
        sumPlot = RiaSummaryTools::parentSummaryPlot(selObj);
    }

    return sumPlot;
}
