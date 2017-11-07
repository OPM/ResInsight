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

#include "RicNewSummaryCrossPlotFeature.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RicEditSummaryPlotFeature.h"
#include "RicSummaryCurveCreator.h"
#include "RicSummaryCurveCreatorDialog.h"

#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryCrossPlotCollection.h"
#include "RimSummaryCurveFilter.h"
#include "RimSummaryPlot.h"

#include "RiuMainPlotWindow.h"

#include "cvfAssert.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicNewSummaryCrossPlotFeature, "RicNewSummaryCrossPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewSummaryCrossPlotFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSummaryCrossPlotFeature::onActionTriggered(bool isChecked)
{
    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

    RimSummaryCrossPlotCollection* summaryCrossPlotColl = project->mainPlotCollection()->summaryCrossPlotCollection();
    RimSummaryPlot* summaryPlot = summaryCrossPlotColl->addSummaryPlot();

    if (summaryPlot)
    {
        summaryCrossPlotColl->updateConnectedEditors();
        summaryPlot->loadDataAndUpdate();

        RiuMainPlotWindow* mainPlotWindow = RiaApplication::instance()->getOrCreateAndShowMainPlotWindow();
        if (mainPlotWindow)
        {
            mainPlotWindow->selectAsCurrentItem(summaryPlot);
            mainPlotWindow->setExpanded(summaryPlot);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSummaryCrossPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Summary Cross Plot");
    actionToSetup->setIcon(QIcon(":/SummaryPlot16x16.png"));
}

