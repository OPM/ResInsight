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

#include "qwt_plot.h"

#include <QObject>

#include <map>

class QEvent;

class RiuQwtPlotWheelZoomer : public QObject
{
    Q_OBJECT
public:
    RiuQwtPlotWheelZoomer( QwtPlot* plot );

    bool eventFilter( QObject* watched, QEvent* event ) override;

    void setAxisIsLogarithmic( QwtPlot::Axis axis, bool logarithmic );

signals:
    void zoomUpdated();

private:
    void zoomOnAxis( QwtPlot* plot, QwtPlot::Axis axis, double zoomFactor, int eventPos );
    bool axisIsLogarithmic( QwtPlot::Axis axis ) const;

private:
    QwtPlot* m_plot;

    std::map<QwtPlot::Axis, bool> m_axesAreLogarithmic;
};
