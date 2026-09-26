/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "gtest/gtest.h"

#include "RiuPlotAxis.h"
#include "RiuQwtPlotWidget.h"

//--------------------------------------------------------------------------------------------------
/// Pruning must keep the axis mapping in sync with the axes owned by qwt. A stale mapping makes
/// QwtPlot::axisScaleDraw() return null, which crashed enableAxisNumberLabels().
//--------------------------------------------------------------------------------------------------
TEST( RiuQwtPlotWidget, PruneAxesKeepsAxisMappingValid )
{
    RiuQwtPlotWidget plotWidget( nullptr );

    plotWidget.pruneAxes( {} );
    plotWidget.ensureAxisIsCreated( RiuPlotAxis( RiaDefines::PlotAxis::PLOT_AXIS_LEFT, 1 ) );
    plotWidget.pruneAxes( {} );

    plotWidget.enableAxis( RiuPlotAxis::defaultLeft(), true );
    EXPECT_TRUE( plotWidget.axisEnabled( RiuPlotAxis::defaultLeft() ) );

    plotWidget.enableAxisNumberLabels( RiuPlotAxis::defaultLeft(), true );
}
