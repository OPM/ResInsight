/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

class RimPlotAxisProperties;
class RimPlotCurve;
class RiuPlotAxis;
class RiuPlotWidget;

#include <QString>
#include <vector>

namespace RimPlotAxisTools
{
void updateVisibleRangesFromPlotWidget( RimPlotAxisProperties* axisProperties, RiuPlotAxis plotAxis, const RiuPlotWidget* const plotWidget );

void updatePlotWidgetFromAxisProperties( RiuPlotWidget*                          plotWidget,
                                         RiuPlotAxis                             plotAxis,
                                         const RimPlotAxisProperties* const      axisProperties,
                                         const QString&                          axisTitle,
                                         const std::vector<const RimPlotCurve*>& plotCurves );

void    applyAxisScaleDraw( RiuPlotWidget* plotWidget, RiuPlotAxis axis, const RimPlotAxisProperties* const axisProperties );
QString scaleFactorText( const RimPlotAxisProperties* const axisProperties );

}; // namespace RimPlotAxisTools
