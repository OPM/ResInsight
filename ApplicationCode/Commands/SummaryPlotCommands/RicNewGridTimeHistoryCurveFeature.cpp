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

#include "RicNewGridTimeHistoryCurveFeature.h"

#include "RiaApplication.h"

#include "RicNewSummaryCurveFeature.h"
#include "WellLogCommands/RicWellLogPlotCurveFeatureImpl.h"

#include "RimGridTimeHistoryCurve.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "RiuMainPlotWindow.h"
#include "RiuSelectionManager.h"

#include "cvfAssert.h"

#include "cvfColor3.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicNewGridTimeHistoryCurveFeature, "RicNewGridTimeHistoryCurveFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewGridTimeHistoryCurveFeature::createCurveFromSelectionItem(const RiuSelectionItem* selectionItem)
{
    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

    RimMainPlotCollection* mainPlotColl = project->mainPlotCollection();
    CVF_ASSERT(mainPlotColl);

    RimSummaryPlotCollection* summaryPlotColl = mainPlotColl->summaryPlotCollection();
    CVF_ASSERT(summaryPlotColl);

    RimSummaryPlot* plot = nullptr;
    if (summaryPlotColl->summaryPlots().size() > 0)
    {
        plot = summaryPlotColl->summaryPlots()[0];
    }

    if (plot)
    {
        RimGridTimeHistoryCurve* newCurve = new RimGridTimeHistoryCurve();
        newCurve->setFromSelectionItem(selectionItem);
        newCurve->setYAxis(RimDefines::PLOT_AXIS_RIGHT);
        newCurve->setLineThickness(2);
        
        cvf::Color3f curveColor = RicWellLogPlotCurveFeatureImpl::curveColorFromTable(plot->curveCount());
        newCurve->setColor(curveColor);

        plot->addGridTimeHistoryCurve(newCurve);

        newCurve->loadDataAndUpdate();

        plot->updateConnectedEditors();

        RiaApplication::instance()->getOrCreateAndShowMainPlotWindow()->selectAsCurrentItem(newCurve);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewGridTimeHistoryCurveFeature::isCommandEnabled()
{
    std::vector<RiuSelectionItem*> items;
    RiuSelectionManager::instance()->selectedItems(items);

    if (items.size() > 0)
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewGridTimeHistoryCurveFeature::onActionTriggered(bool isChecked)
{
    std::vector<RiuSelectionItem*> items;
    RiuSelectionManager::instance()->selectedItems(items);

    for (auto item : items)
    {
        RicNewGridTimeHistoryCurveFeature::createCurveFromSelectionItem(item);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewGridTimeHistoryCurveFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Plot Time History for Selected Cells");
    actionToSetup->setIcon(QIcon(":/SummaryCurve16x16.png"));
}

