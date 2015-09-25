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
#include "RimView.h"
#include "RimProject.h"

#include "RiuMainWindow.h"
#include "RiaApplication.h"

#include "cafSelectionManager.h"

#include <QAction>

#include <vector>


CAF_CMD_SOURCE_INIT(RicNewWellLogCurveExtractionFeature, "RicNewWellLogCurveExtractionFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellLogCurveExtractionFeature::isCommandEnabled()
{
    return (selectedWellLogPlotTrack() != NULL || selectedWellPath() != NULL) && caseAvailable();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellLogCurveExtractionFeature::onActionTriggered(bool isChecked)
{
    RimWellLogPlotTrack* wellLogPlotTrack = selectedWellLogPlotTrack();
    if (wellLogPlotTrack)
    {
        addCurve(wellLogPlotTrack, NULL, NULL);
    }
    else
    {
        RimWellPath* wellPath = selectedWellPath();
        if (wellPath)
        {
            RimWellLogPlotTrack* wellLogPlotTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack();
            RimWellLogExtractionCurve* plotCurve = addCurve(wellLogPlotTrack, RiaApplication::instance()->activeReservoirView(), wellPath);
 
            plotCurve->updatePlotData();
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
RimWellLogPlotTrack* RicNewWellLogCurveExtractionFeature::selectedWellLogPlotTrack() const
{
    std::vector<RimWellLogPlotTrack*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    return selection.size() > 0 ? selection[0] : NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath* RicNewWellLogCurveExtractionFeature::selectedWellPath() const
{
    std::vector<RimWellPath*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    return selection.size() > 0 ? selection[0] : NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellLogCurveExtractionFeature::caseAvailable() const
{
    std::vector<RimCase*> cases;
    RiaApplication::instance()->project()->allCases(cases);

    return cases.size() > 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogExtractionCurve* RicNewWellLogCurveExtractionFeature::addCurve(RimWellLogPlotTrack* plotTrack, RimView* view, RimWellPath* wellPath)
{
    CVF_ASSERT(plotTrack);

    size_t curveIndex = plotTrack->curveCount();

    RimWellLogExtractionCurve* curve = new RimWellLogExtractionCurve();
    plotTrack->addCurve(curve);

    cvf::Color3f curveColor = RicWellLogPlotCurveFeatureImpl::curveColorFromIndex(curveIndex);
    curve->setColor(curveColor);
    curve->setWellPath(wellPath);
    curve->setPropertiesFromView(view);

    plotTrack->updateConnectedEditors();
    RiuMainWindow::instance()->setCurrentObjectInTreeView(curve);

    return curve;
}
