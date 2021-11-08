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
class RimSummaryPlot;
class RiuCvfOverlayItemWidget;
class RiuQwtPlotZoomer;
class RiuQwtPlotWheelZoomer;
class RimPlotAxisPropertiesInterface;
class RiuPlotAnnotationTool;

//==================================================================================================
//
//
//
//==================================================================================================
class RiuSummaryQwtPlot : public RiuQwtPlotWidget
{
    Q_OBJECT;

public:
    RiuSummaryQwtPlot( RimSummaryPlot* plot, QWidget* parent = nullptr );
    ~RiuSummaryQwtPlot();

    void useDateBasedTimeAxis( const QString&                          dateFormat,
                               const QString&                          timeFormat,
                               RiaQDateTimeTools::DateFormatComponents dateComponents = RiaQDateTimeTools::DATE_FORMAT_UNSPECIFIED,
                               RiaQDateTimeTools::TimeFormatComponents timeComponents =
                                   RiaQDateTimeTools::TimeFormatComponents::TIME_FORMAT_UNSPECIFIED );

    void useTimeBasedTimeAxis();
    void setAxisIsLogarithmic( QwtPlot::Axis axis, bool logarithmic );

    void updateAnnotationObjects( RimPlotAxisPropertiesInterface* axisProperties );

protected:
    void contextMenuEvent( QContextMenuEvent* );
    void setDefaults();
    bool isZoomerActive() const override;
    void endZoomOperations() override;

private slots:
    void onZoomedSlot();

private:
    std::unique_ptr<RiuPlotAnnotationTool> m_annotationTool;

    QPointer<RiuQwtPlotZoomer>      m_zoomerLeft;
    QPointer<RiuQwtPlotZoomer>      m_zoomerRight;
    QPointer<RiuQwtPlotWheelZoomer> m_wheelZoomer;
};
