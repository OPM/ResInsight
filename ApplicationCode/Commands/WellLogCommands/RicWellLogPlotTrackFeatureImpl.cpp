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

#include "RimWellLogPlot.h"
#include "RimWellLogPlotTrack.h"
#include "RimWellLogPlotCurve.h"

#include "RiuMainWindow.h"

#include "cafPdmUiTreeView.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellLogPlotTrackFeatureImpl::moveCurvesToWellLogPlotTrack(RimWellLogPlotTrack* destTrack, 
                                                                  const std::vector<RimWellLogPlotCurve*>& curves, 
                                                                  RimWellLogPlotCurve* insertAfterCurve)
{
    CVF_ASSERT(destTrack );

    std::set<RimWellLogPlotTrack*> srcTracks;
    std::set<RimWellLogPlot*> srcPlots;

    for (size_t cIdx = 0; cIdx < curves.size(); cIdx++)
    {
        RimWellLogPlotCurve* curve = curves[cIdx];

        RimWellLogPlotTrack* wellLogPlotTrack;
        curve->firstAnchestorOrThisOfType(wellLogPlotTrack);
        if (wellLogPlotTrack)
        {
            wellLogPlotTrack->removeCurve(curve);
            wellLogPlotTrack->updateConnectedEditors();
            srcTracks.insert(wellLogPlotTrack);
            RimWellLogPlot* plot;
            wellLogPlotTrack->firstAnchestorOrThisOfType(plot);
            if (plot) srcPlots.insert(plot);
        }
    }

    size_t insertionStartIndex = 0;
    if (insertAfterCurve) insertionStartIndex = destTrack->curveIndex(insertAfterCurve) + 1;

    for (size_t cIdx = 0; cIdx < curves.size(); cIdx++)
    {
        destTrack->insertCurve(curves[cIdx], insertionStartIndex + cIdx);
    }

    for (std::set<RimWellLogPlot*>::iterator pIt = srcPlots.begin(); pIt != srcPlots.end(); ++pIt)
    {
        (*pIt)->calculateAvailableDepthRange();
    }

    for (std::set<RimWellLogPlotTrack*>::iterator tIt = srcTracks.begin(); tIt != srcTracks.end(); ++tIt)
    {
        (*tIt)->zoomAllXAndZoomAllDepthOnOwnerPlot();
    }

    destTrack->loadDataAndUpdate();
    destTrack->zoomAllXAndZoomAllDepthOnOwnerPlot();
    destTrack->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellLogPlotTrackFeatureImpl::moveTracksToWellLogPlot(RimWellLogPlot* wellLogPlot, const std::vector<RimWellLogPlotTrack*>& tracks)
{
    CVF_ASSERT(wellLogPlot);

    RimWellLogPlotTrack* wellLogPlotTrack = NULL;

    for (size_t tIdx = 0; tIdx < tracks.size(); tIdx++)
    {
        wellLogPlotTrack = tracks[tIdx];

        RimWellLogPlot* oldPlot;
        wellLogPlotTrack->firstAnchestorOrThisOfType(oldPlot);
        if (oldPlot)
        {
            oldPlot->removeTrack(wellLogPlotTrack);
            oldPlot->updateTrackNames();
            oldPlot->updateConnectedEditors();
        }

        wellLogPlot->insertTrack(wellLogPlotTrack, tIdx);
    }

    wellLogPlot->updateTracks();
    wellLogPlot->updateConnectedEditors();
    
    if (wellLogPlotTrack)
    {
        RiuMainWindow::instance()->projectTreeView()->selectAsCurrentItem(wellLogPlotTrack);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellLogPlotTrackFeatureImpl::moveTracks(RimWellLogPlotTrack* insertAfterTrack, const std::vector<RimWellLogPlotTrack*>& tracks)
{
    CVF_ASSERT(insertAfterTrack);

    RimWellLogPlot* wellLogPlot;
    insertAfterTrack->firstAnchestorOrThisOfType(wellLogPlot);
    if (wellLogPlot)
    {
        wellLogPlot->moveTracks(insertAfterTrack, tracks);
        wellLogPlot->updateConnectedEditors();
    }
}
