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

#include "RiaApplication.h"

#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "RiuMainPlotWindow.h"

#include "WellLogCommands/RicWellLogPlotCurveFeatureImpl.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicNewSummaryCurveFeature, "RicNewSummaryCurveFeature");

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
    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

    RimMainPlotCollection* mainPlotColl = project->mainPlotCollection();
    CVF_ASSERT(mainPlotColl);

    RimSummaryPlotCollection* summaryPlotColl = mainPlotColl->summaryPlotCollection();
    CVF_ASSERT(summaryPlotColl);

    RimSummaryPlot* plot = selectedSummaryPlot();
    if (plot)
    {
        RimSummaryCurve* newCurve = new RimSummaryCurve();
        cvf::Color3f curveColor = RicWellLogPlotCurveFeatureImpl::curveColorFromTable(plot->curveCount());
        newCurve->setColor(curveColor);

        plot->addCurveAndUpdate(newCurve);

        RimSummaryCase* defaultCase = nullptr; 
        if (project->activeOilField()->summaryCaseMainCollection()->summaryCaseCount() > 0)
        {
            defaultCase = project->activeOilField()->summaryCaseMainCollection()->summaryCase(0);
            newCurve->setSummaryCase(defaultCase);

            newCurve->setSummaryAddress(RifEclipseSummaryAddress::fieldVarAddress("FOPT"));

            newCurve->loadDataAndUpdate(true);
        }
        
        plot->updateConnectedEditors();

        RiaApplication::instance()->getOrCreateAndShowMainPlotWindow()->selectAsCurrentItem(newCurve);
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
    caf::PdmObject* selObj =  dynamic_cast<caf::PdmObject*>(caf::SelectionManager::instance()->selectedItem());
    RimSummaryPlot * sumPlot;
    selObj->firstAncestorOrThisOfType(sumPlot);

    return sumPlot;
}
