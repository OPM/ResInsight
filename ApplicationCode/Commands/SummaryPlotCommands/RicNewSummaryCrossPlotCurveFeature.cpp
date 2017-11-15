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

#include "RicNewSummaryCrossPlotCurveFeature.h"

#include "RiaApplication.h"
#include "RiaSummaryTools.h"

#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCrossPlotCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCrossPlot.h"
#include "RimSummaryPlotCollection.h"

#include "RiuMainPlotWindow.h"

#include "WellLogCommands/RicWellLogPlotCurveFeatureImpl.h"

#include "cafSelectionManager.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicNewSummaryCrossPlotCurveFeature, "RicNewSummaryCrossPlotCurveFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewSummaryCrossPlotCurveFeature::isCommandEnabled()
{
    return (selectedCrossPlot());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSummaryCrossPlotCurveFeature::onActionTriggered(bool isChecked)
{
    RimProject* project = RiaApplication::instance()->project();

    RimSummaryCrossPlot* plot = selectedCrossPlot();
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
            newCurve->setSummaryCaseY(defaultCase);

            newCurve->loadDataAndUpdate(true);
        }
        
        plot->updateConnectedEditors();

        RiaApplication::instance()->getOrCreateAndShowMainPlotWindow()->selectAsCurrentItem(newCurve);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSummaryCrossPlotCurveFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Summary Cross Plot Curve");
    actionToSetup->setIcon(QIcon(":/SummaryCurve16x16.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCrossPlot* RicNewSummaryCrossPlotCurveFeature::selectedCrossPlot() const
{
    RimSummaryCrossPlot* crossPlot = nullptr;

    caf::PdmObject* selObj =  dynamic_cast<caf::PdmObject*>(caf::SelectionManager::instance()->selectedItem());
    if (selObj)
    {
        crossPlot = RiaSummaryTools::parentCrossPlot(selObj);
    }

    return crossPlot;
}
