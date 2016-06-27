/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RiuMdiSubWindow.h"

#include "RiaApplication.h"

#include "RimSummaryPlot.h"
#include "RimView.h"
#include "RimWellLogPlot.h"

#include "RiuMainPlotWindow.h"
#include "RiuMainWindow.h"
#include "RiuSummaryQwtPlot.h"
#include "RiuViewer.h"
#include "RiuWellLogPlot.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuMdiSubWindow::RiuMdiSubWindow(QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/) : QMdiSubWindow(parent, flags)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuMdiSubWindow::~RiuMdiSubWindow()
{
    RiuMainWindow::instance()->slotRefreshViewActions();

    if (RiaApplication::instance()->mainPlotWindow())
    {
        RiaApplication::instance()->mainPlotWindow()->slotRefreshViewActions();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMdiSubWindow::closeEvent(QCloseEvent* event)
{
    QWidget* mainWidget = widget();

    RiuWellLogPlot* wellLogPlot = dynamic_cast<RiuWellLogPlot*>(mainWidget);
    RiuSummaryQwtPlot* summaryPlot = dynamic_cast<RiuSummaryQwtPlot*>(mainWidget);
    if (wellLogPlot)
    {
        RiuMainPlotWindow* mainPlotWindow = RiaApplication::instance()->mainPlotWindow();
        CVF_ASSERT(mainPlotWindow);

        wellLogPlot->ownerPlotDefinition()->setMdiWindowGeometry(mainPlotWindow->windowGeometryForWidget(this));
    }
    else if (summaryPlot)
    {
        RiuMainPlotWindow* mainPlotWindow = RiaApplication::instance()->mainPlotWindow();
        CVF_ASSERT(mainPlotWindow);

        summaryPlot->ownerPlotDefinition()->setMdiWindowGeometry(mainPlotWindow->windowGeometryForWidget(this));
    }
    else
    {
        RiuViewer* viewer = mainWidget->findChild<RiuViewer*>();
        if (viewer)
        {
            viewer->ownerReservoirView()->setMdiWindowGeometry(RiuMainWindow::instance()->windowGeometryForWidget(this));
        }
    }

    QMdiSubWindow::closeEvent(event);
}
