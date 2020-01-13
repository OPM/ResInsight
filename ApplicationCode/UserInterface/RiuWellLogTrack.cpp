/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RiuWellLogTrack.h"

#include "RimWellLogTrack.h"

#include "qwt_scale_draw.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_widget.h"

#include <QWheelEvent>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellLogTrack::RiuWellLogTrack( RimWellLogTrack* plotTrackDefinition, QWidget* parent /*= nullptr */ )
    : RiuQwtPlotWidget( parent )
{
    setAxisEnabled( QwtPlot::yLeft, true );
    setAxisEnabled( QwtPlot::yRight, false );
    setAxisEnabled( QwtPlot::xTop, true );
    setAxisEnabled( QwtPlot::xBottom, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellLogTrack::~RiuWellLogTrack() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogTrack::setAxisEnabled( QwtPlot::Axis axis, bool enabled )
{
    if ( enabled )
    {
        enableAxis( axis, true );

        // Align the canvas with the actual min and max values of the curves
        axisScaleEngine( axis )->setAttribute( QwtScaleEngine::Floating, true );
        setAxisScale( axis, 0.0, 100.0 );
        axisScaleDraw( axis )->setMinimumExtent( axisExtent( axis ) );

        axisWidget( axis )->setMargin( 0 );
        setAxisTitleEnabled( axis, true );
    }
    else
    {
        enableAxis( axis, false );
    }
}
