/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RiaRftDefines.h"

#include "cafCmdFeature.h"

class RimWellLogPlot;
class RimSummaryCase;
class RimPlotCurve;
class RimWellLogTrack;

//==================================================================================================
///
//==================================================================================================
class RicNewRftSegmentWellLogPlotFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    static void appendTopologyTrack( RimWellLogPlot* plot, const QString& wellName, RimSummaryCase* summaryCase );
    static void appendPressureTrack( RimWellLogPlot* plot, const QString& wellName, RimSummaryCase* summaryCase );

private:
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;

    void setupActionLook( QAction* actionToSetup ) override;

    static RimPlotCurve* appendTrackAndCurveForBranchType( RimWellLogPlot*           plot,
                                                           const QString&            trackName,
                                                           const QString&            resultName,
                                                           const QString&            wellName,
                                                           RiaDefines::RftBranchType branchType,
                                                           RimSummaryCase*           summaryCase );

    static RimPlotCurve* createAndAddCurve( RimWellLogTrack*          track,
                                            const QString&            resultName,
                                            const QString&            wellName,
                                            RiaDefines::RftBranchType branchType,
                                            RimSummaryCase*           summaryCase );

    static std::vector<RimPlotCurve*>
        appendAdditionalDataSourceTrack( RimWellLogPlot* plot, const QString& wellName, RimSummaryCase* summaryCase );
};
