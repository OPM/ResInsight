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
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMdiSubWindow::closeEvent(QCloseEvent* event)
{
    QWidget* mainWidget = widget();

    RimViewWindow* viewWindow = RiuInterfaceToViewWindow::viewWindowFromWidget(mainWidget);
    if ( viewWindow )
    {
        viewWindow->setMdiWindowGeometry(windowGeometryForWidget(this));
    }
    else
    {
        RiuViewer* viewer = mainWidget->findChild<RiuViewer*>();
        if (viewer)
        {
            viewer->ownerReservoirView()->setMdiWindowGeometry(windowGeometryForWidget(this));
        }
    }
    QMdiSubWindow::closeEvent(event);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimMdiWindowGeometry RiuMdiSubWindow::windowGeometryForWidget(QWidget* widget)
{
    RimMdiWindowGeometry geo;

    // Find topmost parent

    QWidget* nextParent = widget->parentWidget();
    QWidget* parent = nullptr;
    while(nextParent)
    {
        parent = nextParent;
        nextParent = nextParent->parentWidget();
    }

    int mainWinID = 0;
    if (parent)
    {
        if (parent == RiaApplication::instance()->mainPlotWindow())
        {
            mainWinID = 1;
        }
    }

    if (widget)
    {
        geo.mainWindowID = mainWinID;
        geo.x = widget->pos().x();
        geo.y = widget->pos().y();
        geo.width = widget->size().width();
        geo.height = widget->size().height();
        geo.isMaximized = widget->isMaximized();
    }

    return geo;
}
