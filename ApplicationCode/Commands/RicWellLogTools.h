/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#pragma once

#include <QString>

#include <vector>

class RimCase;
class RimSimWellInView;
class Rim3dView;
class Rim3dWellLogCurveCollection;
class RimWellLogExtractionCurve;
class RimWellLogFileChannel;
class RimWellLogFileCurve;
class RimWellLogRftCurve;
class RimWellLogTrack;
class RimWellLogWbsCurve;
class RimWellPath;
class RimWellMeasurementCurve;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RicWellLogTools
{
public:
    static RimSimWellInView* selectedSimulationWell( int* branchIndex );
    static bool              wellHasRftData( const QString& wellName );
    static bool              isWellPathOrSimWellSelectedInView();
    static void              addWellLogChannelsToPlotTrack( RimWellLogTrack*                           plotTrack,
                                                            const std::vector<RimWellLogFileChannel*>& wellLogFileChannels );
    static RimWellPath*      selectedWellPathWithLogFile();
    static RimWellPath*      findWellPathWithLogFileFromSelection();
    static RimWellLogRftCurve*
                                addRftCurve( RimWellLogTrack* plotTrack, const RimSimWellInView* simWell, bool showPlotWindow = true );
    static RimWellLogFileCurve* addFileCurve( RimWellLogTrack* plotTrack, bool showPlotWindow = true );

    static RimWellLogExtractionCurve* addWellLogExtractionCurve( RimWellLogTrack*        plotTrack,
                                                                 RimCase*                rimCase,
                                                                 Rim3dView*              view,
                                                                 RimWellPath*            wellPath,
                                                                 const RimSimWellInView* simWell,
                                                                 int                     branchIndex,
                                                                 bool                    useBranchDetection,
                                                                 bool                    showPlotWindow = true );
    static RimWellLogWbsCurve*        addWellLogWbsCurve( RimWellLogTrack* plotTrack,
                                                          RimCase*         rimCase,
                                                          Rim3dView*       view,
                                                          RimWellPath*     wellPath,
                                                          int              branchIndex,
                                                          bool             useBranchDetection,
                                                          bool             showPlotWindow = true );
    static RimWellMeasurementCurve*   addWellMeasurementCurve( RimWellLogTrack* plotTrack,
                                                               RimWellPath*     wellPath,
                                                               const QString&   measurementName,
                                                               bool             showPlotWindow = true );

private:
    template <typename ExtractionCurveType>
    static ExtractionCurveType* addExtractionCurve( RimWellLogTrack*        plotTrack,
                                                    RimCase*                rimCase,
                                                    Rim3dView*              view,
                                                    RimWellPath*            wellPath,
                                                    const RimSimWellInView* simWell,
                                                    int                     branchIndex,
                                                    bool                    useBranchDetection,
                                                    bool                    showPlotWindow );
};
