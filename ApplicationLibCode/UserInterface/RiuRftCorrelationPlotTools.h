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

#include <functional>
#include <utility>
#include <vector>

class QObject;
class QPoint;
class RiuGroupedBarChartBuilder;
class RiuPlotAxis;
class RiuQwtPlotWidget;

namespace caf
{
class PdmUiItem;
}

//==================================================================================================
/// Qwt specific plot operations used by the RFT correlation plots, kept out of the Rim* classes.
//==================================================================================================
namespace RiuRftCorrelationPlotTools
{
struct PointSeries
{
    std::vector<double> x;
    std::vector<double> y;
    QString             title;
    QColor              color;
    bool                isSelected = false;
};

struct DepthInterval
{
    double top;
    double base;
};

void detachAllItems( RiuQwtPlotWidget* plotWidget );
void attachPointSeries( RiuQwtPlotWidget* plotWidget, const PointSeries& series );
void attachHorizontalLine( RiuQwtPlotWidget* plotWidget, double yValue, Qt::PenStyle style, const QString& label, const QColor& color );
void attachHorizontalBand( RiuQwtPlotWidget* plotWidget, double minValue, double maxValue, const QColor& color );

// Shows the title of the closest curve under the mouse cursor
void installCurveTracker( RiuQwtPlotWidget* plotWidget );

// Selects the item returned by finder when the user clicks near a curve point
void installCurveSelector( RiuQwtPlotWidget* plotWidget, std::function<const caf::PdmUiItem*( const QPoint& )> finder );
int  closestPointIndex( RiuQwtPlotWidget* plotWidget, const QPoint& canvasPos, const std::vector<std::pair<double, double>>& points );

// Reports the depth under a left-button click on the plot canvas. The filter is owned by filterParent.
void installDepthClickFilter( RiuQwtPlotWidget*             plotWidget,
                              const RiuPlotAxis&            depthAxis,
                              bool                          isDepthVertical,
                              std::function<void( double )> callback,
                              QObject*                      filterParent );

// Replaces the thin bars marking depth intervals along the depth axis side of the plot
void setDepthIntervalBars( RiuQwtPlotWidget* plotWidget, const std::vector<DepthInterval>& intervals, bool isDepthVertical );

void addHorizontalBarChart( RiuQwtPlotWidget* plotWidget, RiuGroupedBarChartBuilder& chartBuilder, int maxBarCount );
void detachBarChartItems( RiuQwtPlotWidget* plotWidget );
void removeLegend( RiuQwtPlotWidget* plotWidget );
} // namespace RiuRftCorrelationPlotTools
