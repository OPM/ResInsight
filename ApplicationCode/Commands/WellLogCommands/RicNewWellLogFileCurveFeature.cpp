/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicNewWellLogFileCurveFeature.h"

#include "RicWellLogPlotCurveFeatureImpl.h"
#include "RicNewWellLogPlotFeatureImpl.h"

#include "RimWellLogFileCurve.h"
#include "RimWellLogPlotTrack.h"
#include "RimWellLogFile.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RiuMainWindow.h"

#include "cafSelectionManager.h"

#include <QAction>

#include <vector>


CAF_CMD_SOURCE_INIT(RicNewWellLogFileCurveFeature, "RicNewWellLogFileCurveFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellLogFileCurveFeature::isCommandEnabled()
{
    return selectedWellLogPlotTrack() != NULL || selectedWellPathWithLogFile() != NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellLogFileCurveFeature::onActionTriggered(bool isChecked)
{
    RimWellLogPlotTrack* wellLogPlotTrack = selectedWellLogPlotTrack();
    if (wellLogPlotTrack)
    {
        addCurve(wellLogPlotTrack);
    }
    else
    {
        RimWellPath* wellPath = selectedWellPathWithLogFile();
        if (wellPath)
        {
            RimWellLogPlotTrack* wellLogPlotTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack();
            RimWellLogFileCurve* plotCurve = addCurve(wellLogPlotTrack);
            plotCurve->setWellPath(wellPath);
            plotCurve->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellLogFileCurveFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Well Log LAS Curve");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotTrack* RicNewWellLogFileCurveFeature::selectedWellLogPlotTrack()
{
    std::vector<RimWellLogPlotTrack*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    return selection.size() > 0 ? selection[0] : NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath* RicNewWellLogFileCurveFeature::selectedWellPathWithLogFile()
{
    std::vector<RimWellPath*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    if (selection.size() > 0)
    {
        RimWellPath* wellPath = selection[0];
        if (wellPath->m_wellLogFile())
        {
            return wellPath;
        }
    }

    return NULL;
}
 

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogFileCurve* RicNewWellLogFileCurveFeature::addCurve(RimWellLogPlotTrack* plotTrack)
{
    CVF_ASSERT(plotTrack);

    size_t curveIndex = plotTrack->curveCount();

    RimWellLogFileCurve* curve = new RimWellLogFileCurve();
    plotTrack->addCurve(curve);

    cvf::Color3f curveColor = RicWellLogPlotCurveFeatureImpl::curveColorFromIndex(curveIndex);
    curve->setColor(curveColor);

    plotTrack->updateConnectedEditors();

    RiuMainWindow::instance()->setCurrentObjectInTreeView(curve);

    return curve;
}
