/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include <qwt_date_scale_draw.h>
#include <qwt_date_scale_engine.h>

class RiuQwtDateScaleWrapper
{
public:
    RiuQwtDateScaleWrapper();

    void setFormatStrings( const QString&                          dateFormat,
                           const QString&                          timeFormat,
                           RiaQDateTimeTools::DateFormatComponents dateComponents,
                           RiaQDateTimeTools::TimeFormatComponents timeComponents );

    void    setMaxMajorTicks( int tickCount );
    QString formatStringForRange( const QDateTime& min, const QDateTime& max );

    std::tuple<double, double, int>         adjustedRange( const double& min, const double& max ) const;
    std::vector<std::pair<double, QString>> positionsAndLabels( const double& min, const double& max );

private:
    QwtDateScaleDraw   m_scaleDraw;
    QwtDateScaleEngine m_scaleEngine;
    int                m_maxMajorTicks;
};
