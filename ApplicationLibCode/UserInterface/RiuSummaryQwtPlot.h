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

#include "RiaDateTimeDefines.h"

#include "RiuInterfaceToViewWindow.h"
#include "RiuSummaryPlot.h"

#include <QPointer>

class RimEnsembleParameterColorHandlerInterface;
class RimSummaryPlot;
class RiuCvfOverlayItemWidget;
class RiuQwtPlotZoomer;
class RiuQwtPlotWheelZoomer;
class RimPlotAxisPropertiesInterface;
class RiuPlotAnnotationTool;
class RiuQwtCurvePointTracker;
class RiuQwtPlotWidget;

//==================================================================================================
//
//
//
//==================================================================================================
class RiuSummaryQwtPlot : public RiuSummaryPlot
{
    Q_OBJECT;

public:
    RiuSummaryQwtPlot( RimSummaryPlot* plot, QWidget* parent = nullptr );
    ~RiuSummaryQwtPlot() override;

    void useDateBasedTimeAxis( const QString&                   dateFormat,
                               const QString&                   timeFormat,
                               RiaDefines::DateFormatComponents dateComponents = RiaDefines::DateFormatComponents::DATE_FORMAT_UNSPECIFIED,
                               RiaDefines::TimeFormatComponents timeComponents = RiaDefines::TimeFormatComponents::TIME_FORMAT_UNSPECIFIED ) override;

    void useTimeBasedTimeAxis() override;

    void clearAnnotationObjects() override;
    void updateAnnotationObjects( RimPlotAxisPropertiesInterface* axisProperties ) override;

    RiuPlotWidget* plotWidget() const override;

    void enableCurvePointTracking( bool enable ) override;

protected:
    void setDefaults();
    bool isZoomerActive() const;
    void endZoomOperations();

private slots:
    void onZoomedSlot();

private:
    std::unique_ptr<RiuPlotAnnotationTool> m_annotationTool;
    QPointer<RiuQwtPlotWidget>             m_plotWidget;

    QPointer<RiuQwtPlotZoomer>        m_plotZoomer;
    QPointer<RiuQwtPlotWheelZoomer>   m_wheelZoomer;
    QPointer<RiuQwtCurvePointTracker> m_curvePointTracker;
};
