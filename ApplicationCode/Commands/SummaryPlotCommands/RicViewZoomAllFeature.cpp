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

#include "Rim3dView.h"
#include "RimViewWindow.h"

#include "RiuInterfaceToViewWindow.h"
#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"

#include <QAction>
#include <QMdiSubWindow>

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
    else if (dynamic_cast<RiuPlotMainWindow*>(topLevelWidget))
    {
        RiuPlotMainWindow*    mainPlotWindow = dynamic_cast<RiuPlotMainWindow*>(topLevelWidget);
        QList<QMdiSubWindow*> subwindows     = mainPlotWindow->subWindowList(QMdiArea::StackingOrder);
        if (!subwindows.empty())
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
