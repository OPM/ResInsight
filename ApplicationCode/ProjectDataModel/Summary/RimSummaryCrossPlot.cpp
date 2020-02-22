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

#include "RimSummaryCrossPlot.h"

#include "RimSummaryCurveCollection.h"
#include "RimSummaryPlotSourceStepping.h"

CAF_PDM_SOURCE_INIT( RimSummaryCrossPlot, "SummaryCrossPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCrossPlot::RimSummaryCrossPlot()
{
    CAF_PDM_InitObject( "Summary Cross Plot", ":/SummaryXPlotLight16x16.png", "", "" );

    setAsCrossPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlotSourceStepping* RimSummaryCrossPlot::sourceSteppingObjectForKeyEventHandling() const
{
    return summaryCurveCollection()->sourceSteppingObject( RimSummaryPlotSourceStepping::UNION_X_Y_AXIS );
}
