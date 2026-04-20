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

#include "RimCorrelationBarChartTools.h"

#include "RiuGroupedBarChartBuilder.h"
#include "RiuPlotItem.h"
#include "RiuQwtPlotItem.h"
#include "RiuQwtPlotWidget.h"

#include "qwt_column_symbol.h"
#include "qwt_plot.h"
#include "qwt_plot_barchart.h"
#include "qwt_text.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationBarChartTools::highlightSelectedParameterBar( RiuQwtPlotWidget* plotWidget,
                                                                 const QString&    selectedParamName,
                                                                 const QColor&     barColor,
                                                                 const QColor&     highlightColor )
{
    if ( !plotWidget ) return;

    for ( QwtPlotItem* item : plotWidget->qwtPlot()->itemList( QwtPlotItem::Rtti_PlotBarChart ) )
    {
        auto* barChart = static_cast<QwtPlotBarChart*>( item );
        auto* symbol   = const_cast<QwtColumnSymbol*>( barChart->symbol() );
        if ( !symbol ) continue;

        const QColor color = ( barChart->title().text() == selectedParamName ) ? highlightColor : barColor;

        QPalette palette = symbol->palette();
        palette.setColor( QPalette::Window, color );
        palette.setColor( QPalette::Dark, color );
        symbol->setPalette( palette );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCorrelationBarChartTools::parameterNameFromPlotItem( std::shared_ptr<RiuPlotItem> plotItem )
{
    auto* qwtPlotItem = dynamic_cast<RiuQwtPlotItem*>( plotItem.get() );
    if ( !qwtPlotItem ) return {};

    auto* barChart = dynamic_cast<QwtPlotBarChart*>( qwtPlotItem->qwtPlotItem() );
    if ( !barChart ) return {};

    return barChart->title().text();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationBarChartTools::addCorrelationBar( RiuGroupedBarChartBuilder& chartBuilder,
                                                     const QString&             parameterName,
                                                     double                     correlation,
                                                     bool                       showAbsoluteValues,
                                                     bool                       sortByAbsoluteValues )
{
    const double  value     = showAbsoluteValues ? std::abs( correlation ) : correlation;
    const double  sortValue = sortByAbsoluteValues ? std::abs( value ) : value;
    const QString axisLabel = QString( "%1 (%2)" ).arg( parameterName ).arg( correlation, 5, 'f', 2 );

    // legendText (5th arg) becomes barChart->title() and is used for click-to-select and
    // selection highlight; it must equal parameterName.
    // barText (6th arg) is shown on the axis label and can include the correlation value.
    chartBuilder.addBarEntry( "", "", "", sortValue, parameterName, axisLabel, value );
}
