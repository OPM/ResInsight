/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RiaQDateTimeTools.h"
#include "RiuInterfaceToViewWindow.h"
#include "RiuQwtPlotWidget.h"

#include "cafPdmPointer.h"

#include <QPointer>

class RimEnsembleParameterColorHandlerInterface;
class RiuCvfOverlayItemWidget;
class RiuQwtPlotZoomer;
class RiuQwtPlotWheelZoomer;

//==================================================================================================
//
//
//
//==================================================================================================
class RiuSummaryQwtPlot : public RiuQwtPlotWidget, public RiuInterfaceToViewWindow
{
    Q_OBJECT;

public:
    RiuSummaryQwtPlot( RimPlot* plotDefinition, QWidget* parent = nullptr );
    ~RiuSummaryQwtPlot() override;

    void useDateBasedTimeAxis(
        const QString&                          dateFormat,
        const QString&                          timeFormat,
        RiaQDateTimeTools::DateFormatComponents dateComponents = RiaQDateTimeTools::DATE_FORMAT_UNSPECIFIED,
        RiaQDateTimeTools::TimeFormatComponents timeComponents = RiaQDateTimeTools::TIME_FORMAT_UNSPECIFIED );

    void useTimeBasedTimeAxis();

    RimViewWindow* ownerViewWindow() const override;

    void setLegendFontSize( int fontSize );
    void setLegendVisible( bool visible );

    void setAxisIsLogarithmic( QwtPlot::Axis axis, bool logarithmic );

protected:
    void keyPressEvent( QKeyEvent* ) override;
    void contextMenuEvent( QContextMenuEvent* ) override;
    void setDefaults();
    bool isZoomerActive() const override;
    void endZoomOperations() override;

private slots:
    void onZoomedSlot();

private:
    QPointer<RiuQwtPlotZoomer>      m_zoomerLeft;
    QPointer<RiuQwtPlotZoomer>      m_zoomerRight;
    QPointer<RiuQwtPlotWheelZoomer> m_wheelZoomer;
};
