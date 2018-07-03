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

#include "RicWellLogPlotTrackFeatureImpl.h"

#include "RiaApplication.h"
#include "RiuPlotMainWindow.h"
#include "RiuWellLogPlot.h"
#include "RiuWellLogTrack.h"

#include "RimWellLogCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"

#include "cvfAssert.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellLogPlotTrackFeatureImpl::moveCurvesToWellLogPlotTrack(RimWellLogTrack* destTrack, 
                                                                  const std::vector<RimWellLogCurve*>& curves, 
                                                                  RimWellLogCurve* curveToInsertAfter)
{
    CVF_ASSERT(destTrack );

    std::set<RimWellLogTrack*> srcTracks;
    std::set<RimWellLogPlot*> srcPlots;

    for (size_t cIdx = 0; cIdx < curves.size(); cIdx++)
    {
        RimWellLogCurve* curve = curves[cIdx];

        RimWellLogTrack* wellLogPlotTrack;
        curve->firstAncestorOrThisOfType(wellLogPlotTrack);
        if (wellLogPlotTrack)
        {
            wellLogPlotTrack->takeOutCurve(curve);
            wellLogPlotTrack->updateConnectedEditors();
            srcTracks.insert(wellLogPlotTrack);
            RimWellLogPlot* plot;
            wellLogPlotTrack->firstAncestorOrThisOfType(plot);
            if (plot) srcPlots.insert(plot);
        }
    }

    size_t insertionStartIndex = 0;
    if (curveToInsertAfter) insertionStartIndex = destTrack->curveIndex(curveToInsertAfter) + 1;

    for (size_t cIdx = 0; cIdx < curves.size(); cIdx++)
    {
        destTrack->insertCurve(curves[cIdx], insertionStartIndex + cIdx);
    }

    for (std::set<RimWellLogPlot*>::iterator pIt = srcPlots.begin(); pIt != srcPlots.end(); ++pIt)
    {
        (*pIt)->calculateAvailableDepthRange();
    }

    for (std::set<RimWellLogTrack*>::iterator tIt = srcTracks.begin(); tIt != srcTracks.end(); ++tIt)
    {
        (*tIt)->updateParentPlotZoom();
        (*tIt)->calculateXZoomRangeAndUpdateQwt();
    }

    destTrack->loadDataAndUpdate();
    destTrack->updateParentPlotZoom();
    destTrack->calculateXZoomRangeAndUpdateQwt();
    destTrack->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellLogPlotTrackFeatureImpl::moveTracksToWellLogPlot(RimWellLogPlot* dstWellLogPlot, 
                                                             const std::vector<RimWellLogTrack*>& tracksToMove, 
                                                             RimWellLogTrack* trackToInsertAfter)
{
    CVF_ASSERT(dstWellLogPlot);

    RiuPlotMainWindow* plotWindow = RiaApplication::instance()->getOrCreateMainPlotWindow();

    std::set<RimWellLogPlot*> srcPlots;

    for (size_t tIdx = 0; tIdx < tracksToMove.size(); tIdx++)
    {
        RimWellLogTrack* track = tracksToMove[tIdx];

        RimWellLogPlot* srcPlot;
        track->firstAncestorOrThisOfType(srcPlot);
        if (srcPlot)
        {
            srcPlot->removeTrack(track);
            RiuPlotMainWindow* plotWindow = RiaApplication::instance()->getOrCreateMainPlotWindow();

            srcPlots.insert(srcPlot);
        }
    }

    for (std::set<RimWellLogPlot*>::iterator pIt = srcPlots.begin(); pIt != srcPlots.end(); ++pIt)
    {
        RiuWellLogPlot* viewWidget = dynamic_cast<RiuWellLogPlot*>((*pIt)->viewWidget());
        plotWindow->setWidthOfMdiWindow(viewWidget, viewWidget->preferredSize().width());

        (*pIt)->calculateAvailableDepthRange();
        (*pIt)->updateTrackNames();
        (*pIt)->updateDepthZoom();
        (*pIt)->updateConnectedEditors();
    }


    size_t insertionStartIndex = 0;
    if (trackToInsertAfter) insertionStartIndex = dstWellLogPlot->trackIndex(trackToInsertAfter) + 1;

    for (size_t tIdx = 0; tIdx < tracksToMove.size(); tIdx++)
    {
        dstWellLogPlot->insertTrack(tracksToMove[tIdx], insertionStartIndex + tIdx);
    
    }
    RiuWellLogPlot* viewWidget = dynamic_cast<RiuWellLogPlot*>(dstWellLogPlot->viewWidget());
    plotWindow->setWidthOfMdiWindow(viewWidget, viewWidget->preferredSize().width());

    dstWellLogPlot->updateTrackNames();
    dstWellLogPlot->updateTracks();
    dstWellLogPlot->updateConnectedEditors();
}
