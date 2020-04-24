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
#include <qwt_date.h>

class QwtPlot;
class QwtPlotItem;

class RiuQwtPlotTools
{
public:
    static void setCommonPlotBehaviour( QwtPlot* plot );
    static void setDefaultAxes( QwtPlot* plot );
    static void enableDateBasedBottomXAxis(
        QwtPlot*                                plot,
        const QString&                          dateFormat,
        const QString&                          timeFormat,
        RiaQDateTimeTools::DateFormatComponents dateComponents = RiaQDateTimeTools::DATE_FORMAT_UNSPECIFIED,
        RiaQDateTimeTools::TimeFormatComponents timeComponents = RiaQDateTimeTools::TimeFormatComponents::TIME_FORMAT_UNSPECIFIED );

    static QString dateTimeFormatForInterval( QwtDate::IntervalType                   interval,
                                              const QString&                          dateFormat,
                                              const QString&                          timeFormat,
                                              RiaQDateTimeTools::DateFormatComponents dateComponents,
                                              RiaQDateTimeTools::TimeFormatComponents timeComponents );

    static QwtPlotItem* createBoxShape( const QString& label,
                                        double         startX,
                                        double         endX,
                                        double         startY,
                                        double         endY,
                                        QColor         color,
                                        Qt::BrushStyle brushStyle = Qt::SolidPattern );
};
