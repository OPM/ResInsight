/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include "RiaPlotDefines.h"
#include "Summary/RiaSummaryAddressModifier.h"

#include <vector>

class RimPlotAxisPropertiesInterface;
class RimSummaryCurve;
class RimSummaryPlot;

namespace RimSummaryPlotTools
{
// Assign a Y-axis based on matching unit or summary vector of the other curves and curve sets in the plot. Creates a new
// axis if required.
void assignYPlotAxis( RimSummaryPlot*                                        plot,
                      const std::vector<RimPlotAxisPropertiesInterface*>&    axisProperties,
                      const RiaSummaryAddressModifier::CurveAddressProvider& curveProvider );

// Assign an X-axis for curves using a summary vector as X-axis, based on matching unit or summary vector of the other curves in
// the plot. Creates a new axis if required.
void assignXPlotAxis( RimSummaryPlot* plot, const std::vector<RimPlotAxisPropertiesInterface*>& axisProperties, RimSummaryCurve* curve );
} // namespace RimSummaryPlotTools
