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

#include "qwt_plot_picker.h"
#include "qwt_plot.h"

#include <QPointer>

class QwtPlotMarker;

//--------------------------------------------------------------------------------------------------
/// Class to add mouse over-tracking of curve points with text marker
//--------------------------------------------------------------------------------------------------
class RiuQwtCurvePointTracker : public QwtPlotPicker
{
public:
    explicit RiuQwtCurvePointTracker(QwtPlot* plot, bool isMainAxisHorizontal);
    ~RiuQwtCurvePointTracker();

protected:

    virtual bool      eventFilter(QObject *, QEvent *) override;
    void              removeMarkerOnFocusLeave();

    virtual QwtText   trackerText(const QPoint& pos) const override;
    QPointF           closestCurvePoint(const QPoint& cursorPosition, 
                                        QString* valueAxisValueString, 
                                        QString* mainAxisValueString, 
                                        QwtPlot::Axis* relatedXAxis, 
                                        QwtPlot::Axis* relatedYAxis) const;
    void              updateClosestCurvePointMarker(const QPointF& closestPoint, 
                                                    QwtPlot::Axis relatedXAxis, 
                                                    QwtPlot::Axis relatedYAxis) const;

    QPointer<QwtPlot> m_plot; 
    QwtPlotMarker*    m_plotMarker;
    bool              m_isMainAxisHorizontal;
};

