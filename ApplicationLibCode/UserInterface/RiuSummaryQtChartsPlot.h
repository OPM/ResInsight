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

#include "RiuQtChartsPlotWidget.h"
#include "RiuSummaryPlot.h"

#include <QPointer>

class QWidget;
class RimSummaryPlot;
class RimPlotAxisPropertiesInterface;

//==================================================================================================
//
//
//
//==================================================================================================
class RiuSummaryQtChartsPlot : public RiuSummaryPlot
{
    Q_OBJECT;

public:
    RiuSummaryQtChartsPlot( RimSummaryPlot* plot, QWidget* parent );
    ~RiuSummaryQtChartsPlot() override;

    void useDateBasedTimeAxis(
        const QString&                   dateFormat,
        const QString&                   timeFormat,
        RiaDefines::DateFormatComponents dateComponents = RiaDefines::DateFormatComponents::DATE_FORMAT_UNSPECIFIED,
        RiaDefines::TimeFormatComponents timeComponents = RiaDefines::TimeFormatComponents::TIME_FORMAT_UNSPECIFIED ) override;

    void useTimeBasedTimeAxis() override;

    void updateAnnotationObjects( RimPlotAxisPropertiesInterface* axisProperties ) override;

    RiuPlotWidget* plotWidget() const override;

protected:
    void setDefaults();

private:
    QPointer<RiuQtChartsPlotWidget> m_plotWidget;
};
