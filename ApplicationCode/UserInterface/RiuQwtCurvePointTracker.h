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
class QwtPlotCurve;
class IPlotCurveInfoTextProvider;


//--------------------------------------------------------------------------------------------------
/// Class to add mouse over-tracking of curve points with text marker
//--------------------------------------------------------------------------------------------------
class RiuQwtCurvePointTracker : public QwtPlotPicker
{
public:
    explicit RiuQwtCurvePointTracker(QwtPlot* plot, bool isMainAxisHorizontal, IPlotCurveInfoTextProvider* curveInfoTextProvider = nullptr);
    ~RiuQwtCurvePointTracker() override;

protected:

    bool      eventFilter(QObject *, QEvent *) override;
    void              removeMarkerOnFocusLeave();

    QwtText   trackerText(const QPoint& pos) const override;
    QPointF           closestCurvePoint(const QPoint& cursorPosition,
                                        QString* curveInfoText,
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
    IPlotCurveInfoTextProvider* m_curveInfoTextProvider;
};

//--------------------------------------------------------------------------------------------------
/// Interface for retrieving curve info text
//--------------------------------------------------------------------------------------------------
class IPlotCurveInfoTextProvider
{
public:
    virtual QString curveInfoText(QwtPlotCurve* curve) = 0;
};
