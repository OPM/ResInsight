/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RiuQwtPlotZoomerMultiAxes.h"

#include "qwt_picker_machine.h"
#include "qwt_plot.h"
#include "qwt_scale_div.h"
#include "qwt_scale_map.h"

//--------------------------------------------------------------------------------------------------
///   Create a zoomer for a plot canvas. All axes will be scaled based on the rectangle in screen coordinates.
//--------------------------------------------------------------------------------------------------
RiuQwtPlotZoomerMultiAxes::RiuQwtPlotZoomerMultiAxes( QWidget* canvas, bool doReplot )
    : QwtPlotPicker( canvas )
{
    if ( canvas )
    {
        setTrackerMode( ActiveOnly );
        setRubberBand( RectRubberBand );
        setStateMachine( new QwtPickerDragRectMachine() );

        if ( doReplot && plot() ) plot()->replot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuQwtPlotZoomerMultiAxes::end( bool ok )
{
    ok = QwtPlotPicker::end( ok );
    if ( !ok ) return false;

    QwtPlot* plot = RiuQwtPlotZoomerMultiAxes::plot();
    if ( !plot ) return false;

    const QPolygon& pa = selection();
    if ( pa.count() < 2 ) return false;

    QRect rect = QRect( pa.first(), pa.last() );
    rect       = rect.normalized();

    zoomFromScreenCoords( rect );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotZoomerMultiAxes::zoomFromScreenCoords( const QRectF& screenCoords )
{
    QwtPlot* plot = RiuQwtPlotZoomerMultiAxes::plot();
    if ( !plot ) return;

    const bool doReplot = plot->autoReplot();
    plot->setAutoReplot( false );

    for ( int axisPos = 0; axisPos < QwtAxis::AxisPositions; axisPos++ )
    {
        const int axesCount = plot->axesCount( axisPos );
        for ( int i = 0; i < axesCount; i++ )
        {
            const QwtAxisId axisId( axisPos, i );

            // Get the scale map for the axis used to convert between screen and domain coordinates
            const QwtScaleMap map = plot->canvasMap( axisId );

            if ( axisId.isXAxis() )
            {
                const double screenX1 = screenCoords.left();
                const double screenX2 = screenCoords.right();
                const double domainX1 = map.invTransform( screenX1 );
                const double domainX2 = map.invTransform( screenX2 );

                plot->setAxisScale( axisId, domainX1, domainX2 );
            }
            else
            {
                const double screenY1 = screenCoords.bottom();
                const double screenY2 = screenCoords.top();
                const double domainY1 = map.invTransform( screenY1 );
                const double domainY2 = map.invTransform( screenY2 );

                plot->setAxisScale( axisId, domainY1, domainY2 );
            }
        }
    }

    plot->setAutoReplot( doReplot );
    plot->replot();

    emit zoomed();
}
