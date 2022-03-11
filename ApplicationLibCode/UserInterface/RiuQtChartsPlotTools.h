/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

class RiuQtChartsPlotWidget;

class RiuQtChartsPlotTools
{
public:
    static void setCommonPlotBehaviour( RiuQtChartsPlotWidget* plot );
    static void setDefaultAxes( RiuQtChartsPlotWidget* plot );
    static void enableDateBasedBottomXAxis(
        RiuQtChartsPlotWidget*           plot,
        const QString&                   dateFormat,
        const QString&                   timeFormat,
        RiaDefines::DateFormatComponents dateComponents = RiaDefines::DateFormatComponents::DATE_FORMAT_UNSPECIFIED,
        RiaDefines::TimeFormatComponents timeComponents = RiaDefines::TimeFormatComponents::TIME_FORMAT_UNSPECIFIED );

    static QString dateTimeFormatForInterval( const QString&                   dateFormat,
                                              const QString&                   timeFormat,
                                              RiaDefines::DateFormatComponents dateComponents,
                                              RiaDefines::TimeFormatComponents timeComponents );
};
