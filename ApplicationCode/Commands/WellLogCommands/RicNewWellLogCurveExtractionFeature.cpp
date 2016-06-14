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

#include "RimProject.h"
#include "RimView.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

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
    RimWellLogTrack* wellLogPlotTrack = selectedWellLogPlotTrack();
    if (wellLogPlotTrack)
    {
        addCurve(wellLogPlotTrack, NULL, NULL);
    }
    else
    {
        RimWellPath* wellPath = selectedWellPath();
        if (wellPath)
        {
            RimWellLogTrack* wellLogPlotTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack();
            RimWellLogExtractionCurve* plotCurve = addCurve(wellLogPlotTrack, RiaApplication::instance()->activeReservoirView(), wellPath);

            plotCurve->loadDataAndUpdate();

            RimWellLogPlot* plot = NULL;
            wellLogPlotTrack->firstAnchestorOrThisOfType(plot);
            if (plot && plotCurve->curveData())
            {
                plot->setDepthUnit(plotCurve->curveData()->depthUnit());
            }

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
RimWellLogTrack* RicNewWellLogCurveExtractionFeature::selectedWellLogPlotTrack() const
{
    std::vector<RimWellLogTrack*> selection;
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
RimWellLogExtractionCurve* RicNewWellLogCurveExtractionFeature::addCurve(RimWellLogTrack* plotTrack, RimView* view, RimWellPath* wellPath)
{
    CVF_ASSERT(plotTrack);
    RimWellLogExtractionCurve* curve = new RimWellLogExtractionCurve();

    cvf::Color3f curveColor = RicWellLogPlotCurveFeatureImpl::curveColorFromTable();
    curve->setColor(curveColor);
    curve->setWellPath(wellPath);
    curve->setPropertiesFromView(view);
    plotTrack->addCurve(curve);

    plotTrack->updateConnectedEditors();
    RiuMainWindow::instance()->selectAsCurrentItem(curve);

    return curve;
}
