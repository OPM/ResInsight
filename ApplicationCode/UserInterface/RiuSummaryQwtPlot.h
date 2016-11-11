/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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
#include "cafPdmPointer.h"

#include <QPointer>

class QwtPlotCurve;
class QwtPlotGrid;
class QwtPlotZoomer;
class QwtInterval;
class QwtPicker;
class QwtPlotMarker;

class RimSummaryPlot;

//==================================================================================================
//
//
//
//==================================================================================================
class RiuSummaryQwtPlot : public QwtPlot
{
    Q_OBJECT;
public:
    RiuSummaryQwtPlot(RimSummaryPlot* plotDefinition, QWidget* parent = NULL);
    virtual ~RiuSummaryQwtPlot();

    RimSummaryPlot*                 ownerPlotDefinition();

    void                            currentVisibleWindow(QwtInterval* leftAxis,
                                                         QwtInterval* rightAxis,
                                                         QwtInterval* timeAxis) const;

    void                            setZoomWindow(const QwtInterval& leftAxis,
                                                  const QwtInterval& rightAxis,
                                                  const QwtInterval& timeAxis);

    QPointF                         closestCurvePoint(const QPoint& pos, int* yAxis) const;
    void                            updateClosestCurvePointMarker(const QPointF& pos, int yAxis);

protected:
    virtual bool                    eventFilter(QObject* watched, QEvent* event) override;
    virtual void                    leaveEvent(QEvent *) override;

private:
    void                            setDefaults();
    void                            selectClosestCurve(const QPoint& pos);
    void                            showToolTip(const QPoint& pos);

private slots:
    void                            onZoomedSlot( );
    void                            onAxisClicked(int axis, double value);

private:
    QwtPlotGrid*                    m_grid;
    QwtPicker*                      m_plotPicker;
    QwtPlotMarker*                  m_plotMarker;

    caf::PdmPointer<RimSummaryPlot> m_plotDefinition;

    QPointer<QwtPlotZoomer>         m_zoomerLeft;
    QPointer<QwtPlotZoomer>         m_zoomerRight;
};

