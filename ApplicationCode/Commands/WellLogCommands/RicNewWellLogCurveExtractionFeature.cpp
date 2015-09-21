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

#include "RicNewWellLogCurveExtractionFeature.h"

#include "RicWellLogPlotCurveFeatureImpl.h"
#include "RicNewWellLogPlotFeatureImpl.h"

#include "RimWellLogPlotTrack.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RiuMainWindow.h"

#include "cafSelectionManager.h"

#include <QAction>

#include <vector>


CAF_CMD_SOURCE_INIT(RicNewWellLogCurveExtractionFeature, "RicNewWellLogCurveExtractionFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellLogCurveExtractionFeature::isCommandEnabled()
{
    return selectedWellLogPlotTrack() != NULL || selectedWellPath() != NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellLogCurveExtractionFeature::onActionTriggered(bool isChecked)
{
    RimWellLogPlotTrack* wellLogPlotTrack = selectedWellLogPlotTrack();
    if (wellLogPlotTrack)
    {
        addCurve(wellLogPlotTrack);
    }
    else
    {
        RimWellPath* wellPath = selectedWellPath();
        if (wellPath)
        {
            RimWellLogPlotTrack* wellLogPlotTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack();
            RimWellLogExtractionCurve* plotCurve = addCurve(wellLogPlotTrack);
            plotCurve->setWellPath(wellPath);
            plotCurve->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellLogCurveExtractionFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Well Log Extraction Curve");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotTrack* RicNewWellLogCurveExtractionFeature::selectedWellLogPlotTrack()
{
    std::vector<RimWellLogPlotTrack*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    return selection.size() > 0 ? selection[0] : NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath* RicNewWellLogCurveExtractionFeature::selectedWellPath()
{
    std::vector<RimWellPath*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    return selection.size() > 0 ? selection[0] : NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogExtractionCurve* RicNewWellLogCurveExtractionFeature::addCurve(RimWellLogPlotTrack* plotTrack)
{
    CVF_ASSERT(plotTrack);

    size_t curveIndex = plotTrack->curveCount();

    RimWellLogExtractionCurve* curve = new RimWellLogExtractionCurve();
    plotTrack->addCurve(curve);

    cvf::Color3f curveColor = RicWellLogPlotCurveFeatureImpl::curveColorFromIndex(curveIndex);
    curve->setColor(curveColor);

    plotTrack->updateConnectedEditors();

    RiuMainWindow::instance()->setCurrentObjectInTreeView(curve);

    return curve;
}
