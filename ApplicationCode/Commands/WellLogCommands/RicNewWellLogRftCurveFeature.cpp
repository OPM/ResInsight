/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017  Statoil ASA
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

#include "RicNewWellLogRftCurveFeature.h"

#include "RiaApplication.h"

#include "RimProject.h"
#include "RimWellLogRftCurve.h"
#include "RimWellLogTrack.h"

#include "RiuSelectionManager.h"
#include "RiuMainPlotWindow.h"

#include "cafSelectionManager.h"

#include <QAction>

#include <vector>


CAF_CMD_SOURCE_INIT(RicNewWellLogRftCurveFeature, "RicNewWellLogRftCurveFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellLogRftCurveFeature::isCommandEnabled()
{
    return (selectedWellLogPlotTrack() != nullptr);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellLogRftCurveFeature::onActionTriggered(bool isChecked)
{
    RimWellLogTrack* wellLogPlotTrack = selectedWellLogPlotTrack();
    if (!wellLogPlotTrack) return;

    RicNewWellLogRftCurveFeature::addCurve(wellLogPlotTrack);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellLogRftCurveFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Well Log RFT Curve");
    actionToSetup->setIcon(QIcon(":/WellLogCurve16x16.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogTrack* RicNewWellLogRftCurveFeature::selectedWellLogPlotTrack()
{
    std::vector<RimWellLogTrack*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    return selection.size() > 0 ? selection[0] : nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogRftCurve* RicNewWellLogRftCurveFeature::addCurve(RimWellLogTrack* plotTrack)
{
    CVF_ASSERT(plotTrack);

    RimWellLogRftCurve* curve = new RimWellLogRftCurve();
    plotTrack->addCurve(curve);

    plotTrack->updateConnectedEditors();

    RiuMainPlotWindow* plotwindow = RiaApplication::instance()->getOrCreateAndShowMainPlotWindow();

    RiaApplication::instance()->project()->updateConnectedEditors();

    plotwindow->selectAsCurrentItem(curve);
    
    return curve;
}
