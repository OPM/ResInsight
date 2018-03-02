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

#include "RiuTimeStepChangedHandler.h"

#include "RiuMainWindow.h"
#include "RiuMohrsCirclePlot.h"
#include "RiuPvtPlotPanel.h"
#include "RiuPvtPlotUpdater.h"
#include "RiuRelativePermeabilityPlotPanel.h"
#include "RiuRelativePermeabilityPlotUpdater.h"

#include "Rim3dView.h"

#include "cvfBase.h"
#include "cvfTrace.h"
#include "cvfDebugTimer.h"




//==================================================================================================
///
/// \class RiuTimeStepChangedHandler
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuTimeStepChangedHandler::RiuTimeStepChangedHandler()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuTimeStepChangedHandler* RiuTimeStepChangedHandler::instance()
{
    static RiuTimeStepChangedHandler* singletonInstance = new RiuTimeStepChangedHandler;
    return singletonInstance;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuTimeStepChangedHandler::handleTimeStepChanged(Rim3dView* changedView) const
{
    //cvf::Trace::show("handleTimeStepChanged()  viewName: %s   timeStep:%d", changedView->name().toLatin1().data(), changedView->currentTimeStep());
    //cvf::DebugTimer tim("handleTimeStepChanged()");

    RiuRelativePermeabilityPlotUpdater* relPermPlotUpdater = RiuMainWindow::instance()->relativePermeabilityPlotPanel()->plotUpdater();
    relPermPlotUpdater->updateOnTimeStepChanged(changedView);

    RiuPvtPlotUpdater* pvtPlotUpdater = RiuMainWindow::instance()->pvtPlotPanel()->plotUpdater();
    pvtPlotUpdater->updateOnTimeStepChanged(changedView);

    RiuMohrsCirclePlot* mohrsCirclePlot = RiuMainWindow::instance()->mohrsCirclePlot();
    mohrsCirclePlot->updateOnTimeStepChanged(changedView);

    //tim.reportTimeMS("done");
}
