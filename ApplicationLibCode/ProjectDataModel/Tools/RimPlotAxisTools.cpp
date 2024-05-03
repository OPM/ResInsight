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

#include "RimPlotAxisTools.h"
#include "RimPlotAxisLogRangeCalculator.h"
#include "RimPlotAxisProperties.h"
#include "RimPlotCurve.h"

#include "RiuPlotAxis.h"
#include "RiuPlotWidget.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisTools::updateVisibleRangesFromPlotWidget( RimPlotAxisProperties*     axisProperties,
                                                          RiuPlotAxis                plotAxis,
                                                          const RiuPlotWidget* const plotWidget )
{
    if ( !plotWidget || !axisProperties ) return;

    auto [axisRangeMin, axisRangeMax] = plotWidget->axisRange( plotAxis );

    axisProperties->setVisibleRangeMin( std::min( axisRangeMin, axisRangeMax ) );
    axisProperties->setVisibleRangeMax( std::max( axisRangeMin, axisRangeMax ) );

    axisProperties->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisTools::updatePlotWidgetFromAxisProperties( RiuPlotWidget*                          plotWidget,
                                                           RiuPlotAxis                             axis,
                                                           const RimPlotAxisProperties* const      axisProperties,
                                                           const QString&                          axisTitle,
                                                           const std::vector<const RimPlotCurve*>& plotCurves )
{
    if ( !plotWidget || !axisProperties ) return;

    if ( axisProperties->isActive() )
    {
        plotWidget->enableAxis( axis, true );

        Qt::AlignmentFlag alignment = Qt::AlignCenter;
        if ( axisProperties->titlePosition() == RimPlotAxisPropertiesInterface::AXIS_TITLE_END )
        {
            alignment = Qt::AlignRight;
        }
        plotWidget->setAxisFontsAndAlignment( axis, axisProperties->titleFontSize(), axisProperties->valuesFontSize(), false, alignment );
        if ( !axisTitle.isEmpty() )
        {
            plotWidget->setAxisTitleText( axis, axisTitle );
        }
        plotWidget->setAxisTitleEnabled( axis, true );

        if ( axisProperties->isLogarithmicScaleEnabled() )
        {
            bool isLogScale = plotWidget->axisScaleType( axis ) == RiuPlotWidget::AxisScaleType::LOGARITHMIC;
            if ( !isLogScale )
            {
                plotWidget->setAxisScaleType( axis, RiuPlotWidget::AxisScaleType::LOGARITHMIC );
                plotWidget->setAxisMaxMinor( axis, 5 );
            }

            double min = axisProperties->visibleRangeMin();
            double max = axisProperties->visibleRangeMax();
            if ( axisProperties->isAutoZoom() )
            {
                RimPlotAxisLogRangeCalculator logRangeCalculator( axis.axis(), plotCurves );
                logRangeCalculator.computeAxisRange( &min, &max );
            }

            if ( axisProperties->isAxisInverted() )
            {
                std::swap( min, max );
            }
            plotWidget->setAxisScale( axis, min, max );
        }
        else
        {
            bool isLinearScale = plotWidget->axisScaleType( axis ) == RiuPlotWidget::AxisScaleType::LINEAR;
            if ( !isLinearScale )
            {
                plotWidget->setAxisScaleType( axis, RiuPlotWidget::AxisScaleType::LINEAR );
                plotWidget->setAxisMaxMinor( axis, 3 );
            }

            if ( axisProperties->isAutoZoom() )
            {
                plotWidget->setAxisAutoScale( axis, true );
                plotWidget->setAxisInverted( axis, axisProperties->isAxisInverted() );
            }
            else
            {
                double min = axisProperties->visibleRangeMin();
                double max = axisProperties->visibleRangeMax();
                if ( axisProperties->isAxisInverted() )
                {
                    std::swap( min, max );
                }

                plotWidget->setAxisScale( axis, min, max );
            }
        }
    }
    else
    {
        plotWidget->enableAxis( axis, false );
    }
}
