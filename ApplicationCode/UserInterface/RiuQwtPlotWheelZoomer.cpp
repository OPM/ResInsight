/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RiuQwtPlotWheelZoomer.h"

#include "cvfMath.h"

#include "qwt_plot.h"
#include "qwt_scale_div.h"
#include <QEvent>
#include <QWheelEvent>

#define RIU_LOGARITHMIC_MINIMUM 1.0e-15
#define RIU_SCROLLWHEEL_ZOOMFACTOR 1.1
#define RIU_SCROLLWHEEL_PANFACTOR 0.1

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWheelZoomer::RiuQwtPlotWheelZoomer( QwtPlot* plot )
    : QObject( plot )
    , m_plot( plot )
{
    plot->canvas()->installEventFilter( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWheelZoomer::zoomOnAxis( QwtPlot* plot, QwtPlot::Axis axis, double zoomFactor, int eventPos )
{
    QwtScaleMap scaleMap   = plot->canvasMap( axis );
    double      zoomCenter = scaleMap.invTransform( eventPos );
    double      newMin     = zoomCenter - zoomFactor * ( zoomCenter - scaleMap.s1() );
    double      newMax     = zoomCenter + zoomFactor * ( -zoomCenter + scaleMap.s2() );

    // the QwtScaleDiv::interval yields the current axis range
    // The following thus doesn't limit the zoom to the min/max data but
    // Stops the zoom from changing too much in one step
    QwtInterval axisRange = plot->axisScaleDiv( axis ).interval();
    if ( axisIsLogarithmic( axis ) )
    {
        // Handle inverted axes as well by not assuming maxValue > minValue
        double minValue = std::max( RIU_LOGARITHMIC_MINIMUM, 0.1 * std::min( axisRange.minValue(), axisRange.maxValue() ) );
        double maxValue =
            std::max( RIU_LOGARITHMIC_MINIMUM, 10.0 * std::max( axisRange.minValue(), axisRange.maxValue() ) );

        newMin = cvf::Math::clamp( newMin, minValue, maxValue );
        newMax = cvf::Math::clamp( newMax, minValue, maxValue );
    }

    plot->setAxisScale( axis, newMin, newMax );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuQwtPlotWheelZoomer::axisIsLogarithmic( QwtPlot::Axis axis ) const
{
    auto it = m_axesAreLogarithmic.find( axis );
    return it != m_axesAreLogarithmic.end() ? it->second : false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuQwtPlotWheelZoomer::eventFilter( QObject* watched, QEvent* event )
{
    QWheelEvent* wheelEvent = dynamic_cast<QWheelEvent*>( event );
    if ( wheelEvent )
    {
        double zoomFactor = 1.0 / RIU_SCROLLWHEEL_ZOOMFACTOR;
        if ( wheelEvent->delta() > 0 )
        {
            zoomFactor = RIU_SCROLLWHEEL_ZOOMFACTOR;
        }

        zoomOnAxis( m_plot, QwtPlot::xBottom, zoomFactor, wheelEvent->pos().x() );
        zoomOnAxis( m_plot, QwtPlot::xTop, zoomFactor, wheelEvent->pos().x() );
        zoomOnAxis( m_plot, QwtPlot::yLeft, zoomFactor, wheelEvent->pos().y() );
        zoomOnAxis( m_plot, QwtPlot::yRight, zoomFactor, wheelEvent->pos().y() );

        m_plot->replot();
        emit zoomUpdated();
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWheelZoomer::setAxisIsLogarithmic( QwtPlot::Axis axis, bool logarithmic )
{
    m_axesAreLogarithmic[axis] = logarithmic;
}
