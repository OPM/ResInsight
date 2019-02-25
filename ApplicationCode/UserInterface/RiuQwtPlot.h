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

#include "RiuInterfaceToViewWindow.h"

#include "RimRiuQwtPlotOwnerInterface.h"

#include "cafPdmPointer.h"

#include "qwt_plot.h"

#include <QPointer>

class QwtPlotCurve;
class QwtPlotGrid;
class QwtPlotZoomer;
class QwtInterval;
class QwtPicker;
class QwtPlotMarker;
class QwtScaleWidget;

//==================================================================================================
//
//
//
//==================================================================================================
class RiuQwtPlot : public QwtPlot, public RiuInterfaceToViewWindow
{
    Q_OBJECT;
public:
    RiuQwtPlot(RimViewWindow* viewWindow, QWidget* parent = nullptr);
    ~RiuQwtPlot() override;

    RimRiuQwtPlotOwnerInterface*     ownerPlotDefinition() const;
    RimViewWindow*                   ownerViewWindow() const override;

    QwtInterval             currentAxisRange(QwtPlot::Axis axis);

protected:
    bool                    eventFilter(QObject* watched, QEvent* event) override;

    QSize                   sizeHint() const override;
    QSize                   minimumSizeHint() const override;    

private:
    void                    selectClosestCurve(const QPoint& pos);

private slots:
    void                    onZoomedSlot( );
    void                    onAxisClicked(int axis, double value);

private:
    caf::PdmPointer<RimViewWindow>      m_ownerViewWindow;

    QPointer<QwtPlotZoomer>             m_zoomerLeft;
    QPointer<QwtPlotZoomer>             m_zoomerRight;

};

