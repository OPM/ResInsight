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

#include "RicNewSummaryPlotFeature.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"


#include <QAction>

#include "cvfAssert.h"
#include "RicSummaryCurveCreatorFactoryImpl.h"
#include "RicSummaryCurveCreator.h"
#include "RicSummaryCurveCreatorDialog.h"
#include "RimSummaryPlotCollection.h"
#include "RimSummaryCurveFilter.h"
#include "RiuMainPlotWindow.h"


CAF_CMD_SOURCE_INIT(RicNewSummaryPlotFeature, "RicNewSummaryPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicNewSummaryPlotFeature::RicNewSummaryPlotFeature()
{
    m_curveCreatorFactory = RicSummaryCurveCreatorFactoryImpl::instance();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewSummaryPlotFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSummaryPlotFeature::onActionTriggered(bool isChecked)
{
    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

    auto dialog = m_curveCreatorFactory->dialog();
    auto curveCreator = m_curveCreatorFactory->curveCreator();

    if (!dialog->isVisible())
    {
        dialog->show();
    }
    else
    {
        dialog->raise();
    }

    curveCreator->updateFromSummaryPlot(nullptr);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSummaryPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Summary Plot");
    actionToSetup->setIcon(QIcon(":/SummaryPlot16x16.png"));
}

//--------------------------------------------------------------------------------------------------
/// This method is not called from within this class, only by other classes
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicNewSummaryPlotFeature::createNewSummaryPlot(RimSummaryPlotCollection* summaryPlotColl, RimSummaryCase* summaryCase)
{
    RimSummaryPlot* plot = new RimSummaryPlot();
    summaryPlotColl->summaryPlots().push_back(plot);

    plot->setDescription(QString("Summary Plot %1").arg(summaryPlotColl->summaryPlots.size()));

    RimSummaryCurveFilter* newCurveFilter = new RimSummaryCurveFilter();

    if (summaryCase)
    {
        newCurveFilter->createDefaultCurves(summaryCase, RiaApplication::instance()->preferences()->defaultCurveFilter());
    }

    plot->addCurveFilter(newCurveFilter);

    summaryPlotColl->updateConnectedEditors();
    plot->loadDataAndUpdate();

    RiuMainPlotWindow* mainPlotWindow = RiaApplication::instance()->mainPlotWindow();
    if (mainPlotWindow)
    {
        mainPlotWindow->selectAsCurrentItem(newCurveFilter);
        mainPlotWindow->setExpanded(newCurveFilter, true);
    }

    return plot;
}
