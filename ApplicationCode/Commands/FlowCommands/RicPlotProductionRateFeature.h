/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "cafCmdFeature.h"

#include "RiaDefines.h"
#include "RimFlowDiagSolution.h"

class RimGridSummaryCase;
class RimSimWellInView;
class RimSummaryCurve;
class RimSummaryPlot;

//==================================================================================================
///
//==================================================================================================
class RicPlotProductionRateFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

protected:
    // Overrides
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

private:
    static RimGridSummaryCase* gridSummaryCaseForWell( RimSimWellInView* well );
    static bool                isInjector( RimSimWellInView* well );
    static RimSummaryCurve*    addSummaryCurve( RimSummaryPlot*         plot,
                                                const RimSimWellInView* well,
                                                RimGridSummaryCase*     gridSummaryCase,
                                                const QString&          vectorName,
                                                RiaDefines::PlotAxis    plotAxis,
                                                const cvf::Color3f&     color );
};
