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
#include "qwt_plot.h"
#include <QEvent>
#include <QWheelEvent>

#define RIU_SCROLLWHEEL_ZOOMFACTOR  1.1
#define RIU_SCROLLWHEEL_PANFACTOR   0.1


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWheelZoomer::RiuQwtPlotWheelZoomer(QwtPlot* plot): QObject(plot), m_plot(plot)
{
    plot->canvas()->installEventFilter(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void zoomOnAxis(QwtPlot* plot, QwtPlot::Axis axis, double zoomFactor, int eventPos)
{
    QwtScaleMap scaleMap = plot->canvasMap(axis);
    double zoomCenter = scaleMap.invTransform(eventPos);
    double newMin = zoomCenter - zoomFactor * (zoomCenter - scaleMap.s1());
    double newMax = zoomCenter + zoomFactor * (-zoomCenter + scaleMap.s2());
    plot->setAxisScale(axis, newMin, newMax);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuQwtPlotWheelZoomer::eventFilter(QObject * watched, QEvent * event)
{
    QWheelEvent* wheelEvent = dynamic_cast<QWheelEvent*>(event);
    if ( wheelEvent )
    {
        double zoomFactor = 1.0/RIU_SCROLLWHEEL_ZOOMFACTOR;
        if ( wheelEvent->delta() > 0 )
        {
            zoomFactor = RIU_SCROLLWHEEL_ZOOMFACTOR;
        }

        zoomOnAxis(m_plot, QwtPlot::xBottom, zoomFactor, wheelEvent->pos().x());
        zoomOnAxis(m_plot, QwtPlot::xTop, zoomFactor, wheelEvent->pos().x());
        zoomOnAxis(m_plot, QwtPlot::yLeft, zoomFactor, wheelEvent->pos().y());
        zoomOnAxis(m_plot, QwtPlot::yRight, zoomFactor, wheelEvent->pos().y());

        m_plot->replot();
        emit zoomUpdated();
    }

    return false;
}

