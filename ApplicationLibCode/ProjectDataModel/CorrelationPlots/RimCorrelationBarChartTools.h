/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include <QColor>
#include <QString>

#include <memory>

class RiuGroupedBarChartBuilder;
class RiuPlotItem;
class RiuQwtPlotWidget;

//==================================================================================================
///
/// Presentation helpers shared by the correlation bar/tornado plots (RimCorrelationPlot,
/// RimRftTornadoPlot). The plots rely on the convention that the legendText passed to
/// RiuGroupedBarChartBuilder::addBarEntry equals the parameter name, so the bar chart's
/// title can be used to identify which parameter was clicked or should be highlighted.
///
//==================================================================================================
namespace RimCorrelationBarChartTools
{
void highlightSelectedParameterBar( RiuQwtPlotWidget* plotWidget,
                                    const QString&    selectedParamName,
                                    const QColor&     barColor,
                                    const QColor&     highlightColor );

QString parameterNameFromPlotItem( std::shared_ptr<RiuPlotItem> plotItem );

void addCorrelationBar( RiuGroupedBarChartBuilder& chartBuilder,
                        const QString&             parameterName,
                        double                     correlation,
                        bool                       showAbsoluteValues,
                        bool                       sortByAbsoluteValues );
} // namespace RimCorrelationBarChartTools
