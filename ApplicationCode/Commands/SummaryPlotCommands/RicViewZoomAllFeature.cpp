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

#include "RicViewZoomAllFeature.h"

#include "RiaApplication.h"

#include "RimSummaryPlot.h"
#include "Rim3dView.h"
#include "RimViewWindow.h"
#include "RimWellAllocationPlot.h"
#include "RimWellLogPlot.h"

#include "RiuMainPlotWindow.h"
#include "RiuMainWindow.h"
#include "RiuSummaryQwtPlot.h"
#include "RiuWellAllocationPlot.h"
#include "RiuWellLogPlot.h"

#include <QAction>
#include <QClipboard>
#include <QMdiSubWindow>
#include "RiuFlowCharacteristicsPlot.h"
#include "RimFlowCharacteristicsPlot.h"

CAF_CMD_SOURCE_INIT(RicViewZoomAllFeature, "RicViewZoomAllFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicViewZoomAllFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicViewZoomAllFeature::onActionTriggered(bool isChecked)
{
    this->disableModelChangeContribution();

    QWidget* topLevelWidget = RiaApplication::activeWindow();

    if (dynamic_cast<RiuMainWindow*>(topLevelWidget))
    {
        RimViewWindow* viewWindow = RiaApplication::instance()->activeReservoirView();
        viewWindow->zoomAll();
    }
    else if (dynamic_cast<RiuMainPlotWindow*>(topLevelWidget))
    {
        RiuMainPlotWindow* mainPlotWindow = dynamic_cast<RiuMainPlotWindow*>(topLevelWidget);
        QList<QMdiSubWindow*> subwindows = mainPlotWindow->subWindowList(QMdiArea::StackingOrder);
        if (subwindows.size() > 0)
        {
            RimViewWindow* viewWindow = RiuInterfaceToViewWindow::viewWindowFromWidget(subwindows.back()->widget());

            if (viewWindow)
            {
                viewWindow->zoomAll();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicViewZoomAllFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Zoom All");
    actionToSetup->setIcon(QIcon(":/ZoomAll16x16.png"));
}

