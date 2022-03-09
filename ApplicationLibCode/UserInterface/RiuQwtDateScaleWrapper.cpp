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

#include "RiuQwtDateScaleWrapper.h"
#include "RiuQwtPlotTools.h"

#include <set>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtDateScaleWrapper::RiuQwtDateScaleWrapper()
    : m_scaleEngine( Qt::UTC )
    , m_maxMajorTicks( 7 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtDateScaleWrapper::setFormatStrings( const QString&                          dateFormat,
                                               const QString&                          timeFormat,
                                               RiaQDateTimeTools::DateFormatComponents dateComponents,
                                               RiaQDateTimeTools::TimeFormatComponents timeComponents )
{
    std::set<QwtDate::IntervalType> intervals = { QwtDate::Year,
                                                  QwtDate::Month,
                                                  QwtDate::Week,
                                                  QwtDate::Day,
                                                  QwtDate::Hour,
                                                  QwtDate::Minute,
                                                  QwtDate::Second,
                                                  QwtDate::Millisecond };

    for ( QwtDate::IntervalType interval : intervals )
    {
        m_scaleDraw.setDateFormat( interval,
                                   RiuQwtPlotTools::dateTimeFormatForInterval( interval,
                                                                               dateFormat,
                                                                               timeFormat,
                                                                               dateComponents,
                                                                               timeComponents ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtDateScaleWrapper::setMaxMajorTicks( int tickCount )
{
    m_maxMajorTicks = tickCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::tuple<double, double, int> RiuQwtDateScaleWrapper::adjustedRange( const double& min, const double& max ) const
{
    double stepSize = 0.0;

    double adjustedMin = min;
    double adjustedMax = max;
    m_scaleEngine.autoScale( m_maxMajorTicks, adjustedMin, adjustedMax, stepSize );

    auto scaleDiv = m_scaleEngine.divideScale( adjustedMin, adjustedMax, m_maxMajorTicks, 0 );
    auto ticks    = scaleDiv.ticks( QwtScaleDiv::MajorTick );

    return { adjustedMin, adjustedMax, ticks.size() - 1 };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuQwtDateScaleWrapper::formatStringForRange( const QDateTime& min, const QDateTime& max )
{
    auto intervalType = m_scaleEngine.intervalType( min, max, m_maxMajorTicks );
    auto dateFormat   = m_scaleDraw.dateFormat( intervalType );

    return dateFormat;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<double, QString>> RiuQwtDateScaleWrapper::positionsAndLabels( const double& min, const double& max )
{
    double stepSize = 0.0;

    double adjustedMin = min;
    double adjustedMax = max;
    m_scaleEngine.autoScale( m_maxMajorTicks, adjustedMin, adjustedMax, stepSize );

    auto scaleDiv = m_scaleEngine.divideScale( adjustedMin, adjustedMax, m_maxMajorTicks, 0 );
    auto ticks    = scaleDiv.ticks( QwtScaleDiv::MajorTick );

    m_scaleDraw.setScaleDiv( scaleDiv );

    auto formatString =
        formatStringForRange( QDateTime::fromMSecsSinceEpoch( min ), QDateTime::fromMSecsSinceEpoch( max ) );

    std::vector<std::pair<double, QString>> valueAndLabel;
    for ( auto t : ticks )
    {
        auto qwtLabel  = m_scaleDraw.label( t );
        auto labelText = QDateTime::fromMSecsSinceEpoch( t ).toString( formatString );

        valueAndLabel.emplace_back( t, labelText );
    }

    return valueAndLabel;
}
