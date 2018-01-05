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

#include "RicShowPlotDataFeature.h"

#include "RiaApplication.h"

#include "RimProject.h"
#include "RimSummaryCrossPlot.h"
#include "RimSummaryPlot.h"
#include "RimWellLogPlot.h"

#include "RiuMainPlotWindow.h"
#include "RiuTextDialog.h"

#include "cafSelectionManagerTools.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicShowPlotDataFeature, "RicShowPlotDataFeature");


//--------------------------------------------------------------------------------------------------
///
///
/// RicShowPlotDataFeature
/// 
/// 
//--------------------------------------------------------------------------------------------------
bool RicShowPlotDataFeature::isCommandEnabled()
{
    auto selectedSummaryPlots = caf::selectedObjectsByType<RimSummaryPlot*>();
    if (selectedSummaryPlots.size() > 0)
    {
        for (auto c : selectedSummaryPlots)
        {
            if (dynamic_cast<RimSummaryCrossPlot*>(c))
            {
                return false;
            }
        }

        return true;
    }

    auto wellLogPlots = caf::selectedObjectsByType<RimWellLogPlot*>();
    if (wellLogPlots.size() > 0) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowPlotDataFeature::onActionTriggered(bool isChecked)
{
    this->disableModelChangeContribution();

    std::vector<RimSummaryPlot*> selectedSummaryPlots = caf::selectedObjectsByType<RimSummaryPlot*>();
    std::vector<RimWellLogPlot*> wellLogPlots = caf::selectedObjectsByType<RimWellLogPlot*>();

    if (selectedSummaryPlots.size() == 0 && wellLogPlots.size() == 0)
    {
        CVF_ASSERT(false);

        return;
    }

    RiuMainPlotWindow* plotwindow = RiaApplication::instance()->mainPlotWindow();
    CVF_ASSERT(plotwindow);

    for (RimSummaryPlot* summaryPlot : selectedSummaryPlots)
    {
        QString title = summaryPlot->description();
        QString text = summaryPlot->asciiDataForPlotExport();

        RicShowPlotDataFeature::showTextWindow(title, text);
    }

    for (RimWellLogPlot* wellLogPlot : wellLogPlots)
    {
        QString title = wellLogPlot->description();
        QString text = wellLogPlot->asciiDataForPlotExport();

        RicShowPlotDataFeature::showTextWindow(title, text);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowPlotDataFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Show Plot Data");
    actionToSetup->setIcon(QIcon(":/PlotWindow24x24.png"));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowPlotDataFeature::showTextWindow(const QString& title, const QString& text)
{
    RiuMainPlotWindow* plotwindow = RiaApplication::instance()->mainPlotWindow();
    CVF_ASSERT(plotwindow);

    RiuTextDialog* textWiget = new RiuTextDialog();
    textWiget->setMinimumSize(400, 600);

    textWiget->setWindowTitle(title);
    textWiget->setText(text);

    textWiget->show();

    plotwindow->addToTemporaryWidgets(textWiget);
}
