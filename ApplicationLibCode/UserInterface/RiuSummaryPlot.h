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

class RimSummaryPlot;
class RimPlotAxisPropertiesInterface;
class RiuPlotWidget;

//==================================================================================================
//
//
//
//==================================================================================================
class RiuSummaryPlot : public QObject
{
    Q_OBJECT
public:
    RiuSummaryPlot( RimSummaryPlot* plot, QWidget* parent );
    ~RiuSummaryPlot() override;

    virtual void useDateBasedTimeAxis(
        const QString&                          dateFormat,
        const QString&                          timeFormat,
        RiaQDateTimeTools::DateFormatComponents dateComponents = RiaQDateTimeTools::DATE_FORMAT_UNSPECIFIED,
        RiaQDateTimeTools::TimeFormatComponents timeComponents =
            RiaQDateTimeTools::TimeFormatComponents::TIME_FORMAT_UNSPECIFIED ) = 0;

    virtual void useTimeBasedTimeAxis() = 0;

    virtual void updateAnnotationObjects( RimPlotAxisPropertiesInterface* axisProperties ) = 0;

    virtual RiuPlotWidget* plotWidget() const = 0;
};
