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
#include "RimSummaryPlotAxisFormatter.h"

#include "RiuPlotAxis.h"
#include "RiuPlotWidget.h"
#include "RiuQwtPlotWidget.h"

#include "qwt_plot.h"
#include "qwt_scale_draw.h"
#include "qwt_text.h"

#include <cmath>

namespace RimPlotAxisTools
{

//--------------------------------------------------------------------------------------------------
// e    format as [-]9.9e[+|-]999
// E    format as[-]9.9E[+| -]999
// f    format as[-]9.9
// g    use e or f format, whichever is the most concise
// G    use E or f format, whichever is the most concise

//--------------------------------------------------------------------------------------------------
class SummaryScaleDraw : public QwtScaleDraw
{
public:
    SummaryScaleDraw( double                                  scaleFactor,
                      int                                     numberOfDecimals,
                      RimPlotAxisProperties::NumberFormatType numberFormat = RimPlotAxisProperties::NUMBER_FORMAT_AUTO )
    {
        m_scaleFactor      = scaleFactor;
        m_numberOfDecimals = numberOfDecimals;
        m_numberFormat     = numberFormat;
    }

    QwtText label( double value ) const override
    {
        if ( qFuzzyCompare( scaledValue( value ) + 1.0, 1.0 ) ) value = 0.0;

        return QString::number( scaledValue( value ), numberFormat(), m_numberOfDecimals );
    }

private:
    char numberFormat() const
    {
        switch ( m_numberFormat )
        {
            case RimPlotAxisProperties::NUMBER_FORMAT_AUTO:
                return 'g';
            case RimPlotAxisProperties::NUMBER_FORMAT_DECIMAL:
                return 'f';
            case RimPlotAxisProperties::NUMBER_FORMAT_SCIENTIFIC:
                return 'e';
            default:
                return 'g';
        }
    }

    double scaledValue( double value ) const { return value / m_scaleFactor; }

private:
    double                                  m_scaleFactor;
    int                                     m_numberOfDecimals;
    RimPlotAxisProperties::NumberFormatType m_numberFormat;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void updateVisibleRangesFromPlotWidget( RimPlotAxisProperties* axisProperties, RiuPlotAxis plotAxis, const RiuPlotWidget* const plotWidget )
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
void updatePlotWidgetFromAxisProperties( RiuPlotWidget*                          plotWidget,
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
            plotWidget->setAxisTitleText( axis, axisTitle + RimPlotAxisTools::scaleFactorText( axisProperties ) );
        }
        plotWidget->setAxisTitleEnabled( axis, true );

        applyAxisScaleDraw( plotWidget, axis, axisProperties );

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void applyAxisScaleDraw( RiuPlotWidget* plotWidget, RiuPlotAxis axis, const RimPlotAxisProperties* const axisProperties )
{
    if ( auto qwtPlotWidget = dynamic_cast<RiuQwtPlotWidget*>( plotWidget ) )
    {
        auto qwtAxisId = qwtPlotWidget->toQwtPlotAxis( axis );

        if ( axisProperties->numberFormat() == RimPlotAxisProperties::NUMBER_FORMAT_AUTO && axisProperties->scaleFactor() == 1.0 )
        {
            // Default to Qwt's own scale draw to avoid changing too much for default values
            qwtPlotWidget->qwtPlot()->setAxisScaleDraw( qwtAxisId, new QwtScaleDraw );
        }
        else
        {
            qwtPlotWidget->qwtPlot()->setAxisScaleDraw( qwtAxisId,
                                                        new SummaryScaleDraw( axisProperties->scaleFactor(),
                                                                              axisProperties->decimalCount(),
                                                                              axisProperties->numberFormat() ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString scaleFactorText( const RimPlotAxisProperties* const axisProperties )
{
    if ( axisProperties->scaleFactor() != 1.0 )
    {
        int exponent = std::log10( axisProperties->scaleFactor() );
        return QString( " x 10<sup>%1</sup> " ).arg( QString::number( exponent ) );
    }

    return {};
}

} // namespace RimPlotAxisTools
