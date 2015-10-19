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
void RicWellLogPlotTrackFeatureImpl::moveCurvesToWellLogPlotTrack(RimWellLogPlotTrack* wellLogPlotTrack, const std::vector<RimWellLogPlotCurve*>& curves)
{
    CVF_ASSERT(wellLogPlotTrack);

    for (size_t cIdx = 0; cIdx < curves.size(); cIdx++)
    {
        RimWellLogPlotTrack* oldPlotTrack;
        curves[cIdx]->firstAnchestorOrThisOfType(oldPlotTrack);
        if (oldPlotTrack)
        {
            oldPlotTrack->removeCurve(curves[cIdx]);
            oldPlotTrack->updateConnectedEditors();
        }

        wellLogPlotTrack->addCurve(curves[cIdx]);
    }

    wellLogPlotTrack->updateAxisRangesAndReplot();
    wellLogPlotTrack->updateConnectedEditors();
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
            oldPlot->updateConnectedEditors();
        }

        wellLogPlot->addTrack(wellLogPlotTrack);
    }

    wellLogPlot->updateTracks();
    wellLogPlot->updateConnectedEditors();
    
    if (wellLogPlotTrack)
    {
        RiuMainWindow::instance()->projectTreeView()->selectAsCurrentItem(wellLogPlotTrack);
    }
}
