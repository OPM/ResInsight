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

#include "RiaGuiApplication.h"
#include "RiuPlotMainWindow.h"
#include "RiuQwtPlotWidget.h"
#include "RiuWellLogPlot.h"

#include "RimWellLogCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"

#include "cvfAssert.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellLogPlotTrackFeatureImpl::moveCurvesToWellLogPlotTrack( RimWellLogTrack*                     destTrack,
                                                                   const std::vector<RimWellLogCurve*>& curves,
                                                                   int insertAtPosition )
{
    CVF_ASSERT( destTrack );

    std::set<RimWellLogTrack*> srcTracks;
    std::set<RimWellLogPlot*>  srcPlots;

    for ( size_t cIdx = 0; cIdx < curves.size(); cIdx++ )
    {
        RimWellLogCurve* curve = curves[cIdx];

        RimWellLogTrack* wellLogPlotTrack;
        curve->firstAncestorOrThisOfType( wellLogPlotTrack );
        if ( wellLogPlotTrack )
        {
            wellLogPlotTrack->removeCurve( curve );
            srcTracks.insert( wellLogPlotTrack );
            RimWellLogPlot* plot;
            wellLogPlotTrack->firstAncestorOrThisOfType( plot );
            if ( plot ) srcPlots.insert( plot );
        }
    }

    for ( size_t cIdx = 0; cIdx < curves.size(); cIdx++ )
    {
        if ( insertAtPosition >= 0 )
        {
            size_t position = (size_t)insertAtPosition + cIdx;
            destTrack->insertCurve( curves[cIdx], position );
        }
        else
        {
            destTrack->addCurve( curves[cIdx] );
        }
    }

    for ( auto track : srcTracks )
    {
        track->setAutoScaleXEnabled( true );
        track->updateParentPlotZoom();
        track->updateStackedCurveData();
        track->updateConnectedEditors();
    }

    for ( auto plot : srcPlots )
    {
        plot->calculateAvailableDepthRange();
    }

    destTrack->loadDataAndUpdate();
    destTrack->updateStackedCurveData();
    destTrack->setAutoScaleXEnabled( true );
    destTrack->updateParentPlotZoom();
    destTrack->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellLogPlotTrackFeatureImpl::moveTracksToWellLogPlot( RimWellLogPlot*                      wellLogPlot,
                                                              const std::vector<RimWellLogTrack*>& tracksToMove,
                                                              int                                  insertAtPosition )
{
    CVF_ASSERT( wellLogPlot );

    for ( size_t tIdx = 0; tIdx < tracksToMove.size(); tIdx++ )
    {
        RimWellLogTrack* plot      = tracksToMove[tIdx];
        caf::PdmObject*  pdmObject = dynamic_cast<caf::PdmObject*>( plot );
        RimWellLogPlot*  srcPlot;
        pdmObject->firstAncestorOrThisOfType( srcPlot );
        if ( srcPlot )
        {
            srcPlot->removePlot( plot );
        }
    }

    for ( size_t tIdx = 0; tIdx < tracksToMove.size(); tIdx++ )
    {
        if ( insertAtPosition >= 0 )
        {
            wellLogPlot->insertPlot( tracksToMove[tIdx], (size_t)insertAtPosition + tIdx );
        }
        else
        {
            wellLogPlot->addPlot( tracksToMove[tIdx] );
        }
    }
}
