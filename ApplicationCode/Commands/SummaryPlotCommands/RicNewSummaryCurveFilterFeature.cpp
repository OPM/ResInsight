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

#include "RicNewSummaryCurveFilterFeature.h"

#include "RiaApplication.h"

#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveFilter.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "RiuMainPlotWindow.h"

#include "WellLogCommands/RicWellLogPlotCurveFeatureImpl.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicNewSummaryCurveFilterFeature, "RicNewSummaryCurveFilterFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewSummaryCurveFilterFeature::isCommandEnabled()
{
    return (selectedSummaryPlot());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSummaryCurveFilterFeature::onActionTriggered(bool isChecked)
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
        RimSummaryCurveFilter* newCurveFilter = new RimSummaryCurveFilter();
        plot->addCurveFilter(newCurveFilter);
        
        plot->updateConnectedEditors();

        RiaApplication::instance()->getOrCreateAndShowMainPlotWindow()->selectAsCurrentItem(newCurveFilter);
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSummaryCurveFilterFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Summary Curve Filter");
    actionToSetup->setIcon(QIcon(":/SummaryCurveFilter16x16.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicNewSummaryCurveFilterFeature::selectedSummaryPlot() const
{
    caf::PdmObject* selObj =  dynamic_cast<caf::PdmObject*>(caf::SelectionManager::instance()->selectedItem());
    RimSummaryPlot * sumPlot;
    selObj->firstAncestorOrThisOfType(sumPlot);

    return sumPlot;
}
