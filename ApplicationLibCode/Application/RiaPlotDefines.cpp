/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RiaPlotDefines.h"

#include "cafAppEnum.h"

namespace caf
{
template <>
void caf::AppEnum<RiaDefines::PlotAxis>::setUp()
{
    addItem( RiaDefines::PlotAxis::PLOT_AXIS_LEFT, "PLOT_AXIS_LEFT", "Left" );
    addItem( RiaDefines::PlotAxis::PLOT_AXIS_RIGHT, "PLOT_AXIS_RIGHT", "Right" );
    addItem( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM, "PLOT_AXIS_BOTTOM", "Bottom" );
    addItem( RiaDefines::PlotAxis::PLOT_AXIS_TOP, "PLOT_AXIS_TOP", "Top" );

    setDefault( RiaDefines::PlotAxis::PLOT_AXIS_LEFT );
}
}; // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaDefines::minimumDefaultValuePlot()
{
    return -10.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaDefines::minimumDefaultLogValuePlot()
{
    return 1.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaDefines::maximumDefaultValuePlot()
{
    return 100.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaDefines::isHorizontal( RiaDefines::PlotAxis axis )
{
    return !isVertical( axis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaDefines::isVertical( RiaDefines::PlotAxis axis )
{
    return ( axis == RiaDefines::PlotAxis::PLOT_AXIS_LEFT || axis == RiaDefines::PlotAxis::PLOT_AXIS_RIGHT );
}
