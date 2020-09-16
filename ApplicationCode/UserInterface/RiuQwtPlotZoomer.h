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
#pragma once

#include "RiuGuiTheme.h"

#include "qwt_plot_zoomer.h"

#include <QMouseEvent>

class RiuQwtPlotZoomer : public QwtPlotZoomer
{
public:
    RiuQwtPlotZoomer( QWidget* canvas, bool doReplot = true )
        : QwtPlotZoomer( canvas, doReplot )
    {
        auto color = RiuGuiTheme::getColorByVariableName( "markerColor" );

        setRubberBandPen( color );
        setTrackerPen( color );
    }

    bool isActiveAndValid() const
    {
        if ( !isActive() ) return false;

        auto currentSelection = selection();
        return accept( currentSelection );
    }

    void endZoomOperation() { reset(); }

protected:
    QSizeF minZoomSize() const override { return QwtPlotZoomer::minZoomSize() / 10.0e6; }

    bool accept( QPolygon& pa ) const override
    {
        if ( pa.count() < 2 ) return false;

        QRect rect = QRect( pa[0], pa[int( pa.count() ) - 1] );
        rect       = rect.normalized();

        // This size is larger than the minSize value in the base class
        const int minSize = 10;
        if ( rect.width() < minSize && rect.height() < minSize ) return false;

        return QwtPlotZoomer::accept( pa );
    }
};
