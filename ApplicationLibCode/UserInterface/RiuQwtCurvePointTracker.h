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
#include "qwt_plot_picker.h"

#include <QPointer>

class QwtPlotMarker;
class QwtPlotCurve;
class RiuPlotCurveInfoTextProvider;

//--------------------------------------------------------------------------------------------------
/// Class to add mouse over-tracking of curve points with text marker
//--------------------------------------------------------------------------------------------------
class RiuQwtCurvePointTracker : public QwtPlotPicker
{
public:
    explicit RiuQwtCurvePointTracker( QwtPlot*                      plot,
                                      bool                          isMainAxisHorizontal,
                                      RiuPlotCurveInfoTextProvider* curveInfoTextProvider = nullptr );
    ~RiuQwtCurvePointTracker() override;

protected:
    bool eventFilter( QObject*, QEvent* ) override;
    void removeMarkerOnFocusLeave();

    QwtText trackerText( const QPoint& pos ) const override;
    QPointF closestCurvePoint( const QPoint& cursorPosition,
                               QString*      curveInfoText,
                               QString*      valueAxisValueString,
                               QString*      mainAxisValueString,
                               QwtAxisId*    relatedXAxis,
                               QwtAxisId*    relatedYAxis ) const;
    void updateClosestCurvePointMarker( const QPointF& closestPoint, QwtAxisId relatedXAxis, QwtAxisId relatedYAxis ) const;

    QPointer<QwtPlot>             m_plot;
    QwtPlotMarker*                m_plotMarker;
    bool                          m_isMainAxisHorizontal;
    RiuPlotCurveInfoTextProvider* m_curveInfoTextProvider;
};
