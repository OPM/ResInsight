/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "cafAppEnum.h"

#include <qglobal.h>
#include <qnamespace.h>

#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QVariant>

#include <map>
#include <string>
#include <vector>

class QDateTime;
class QDate;
class QTime;
class DateTimeSpan;

namespace caf
{
class PdmOptionItemInfo;
};

//==================================================================================================
//
//==================================================================================================
class RiaQDateTimeTools
{
    static const DateTimeSpan TIMESPAN_DAY;
    static const DateTimeSpan TIMESPAN_WEEK;
    static const DateTimeSpan TIMESPAN_MONTH;
    static const DateTimeSpan TIMESPAN_QUARTER;
    static const DateTimeSpan TIMESPAN_HALFYEAR;
    static const DateTimeSpan TIMESPAN_YEAR;
    static const DateTimeSpan TIMESPAN_DECADE;

public:
    enum DateFormatComponents
    {
        DATE_FORMAT_UNSPECIFIED = -2,
        DATE_FORMAT_NONE        = -1,
        DATE_FORMAT_YEAR        = 0,
        DATE_FORMAT_YEAR_MONTH,
        DATE_FORMAT_YEAR_MONTH_DAY,
        DATE_FORMAT_SIZE
    };

    enum class TimeFormatComponents
    {
        TIME_FORMAT_UNSPECIFIED = -2,
        TIME_FORMAT_NONE        = -1,
        TIME_FORMAT_HOUR,
        TIME_FORMAT_HOUR_MINUTE,
        TIME_FORMAT_HOUR_MINUTE_SECOND,
        TIME_FORMAT_HOUR_MINUTE_SECOND_MILLISECOND,
        TIME_FORMAT_SIZE
    };

    enum class DateTimePeriod
    {
        NONE = -1,
        DAY,
        WEEK,
        MONTH,
        QUARTER,
        HALFYEAR,
        YEAR,
        DECADE
    };
    using DateTimePeriodEnum = caf::AppEnum<DateTimePeriod>;

    static Qt::TimeSpec currentTimeSpec();

    static QDateTime fromString( const QString& dateString, const QString& format );
    static QDateTime fromYears( double years );
    static QDateTime fromTime_t( time_t t );

    static QDateTime addMSecs( const QDateTime& dt, double msecs );
    static QDateTime addDays( const QDateTime& dt, double days );
    static QDateTime addYears( const QDateTime& dt, double years );
    static QDateTime addSpan( const QDateTime& dt, DateTimeSpan span );
    static QDateTime subtractSpan( const QDateTime& dt, DateTimeSpan span );
    static QDateTime addPeriod( const QDateTime& dt, RiaQDateTimeTools::DateTimePeriod period );
    static QDateTime subtractPeriod( const QDateTime& dt, RiaQDateTimeTools::DateTimePeriod period );

    static QDateTime epoch();

    static QDateTime createUtcDateTime();
    static QDateTime createUtcDateTime( const QDate& date );
    static QDateTime createUtcDateTime( const QDate& date, const QTime& time );
    static QDateTime createUtcDateTime( const QDateTime& dt );

    static bool lessThan( const QDateTime& dt1, const QDateTime& dt2 );

    static const DateTimeSpan timeSpan( RiaQDateTimeTools::DateTimePeriod period );
    static QDateTime          truncateTime( const QDateTime& dt, RiaQDateTimeTools::DateTimePeriod period );

    static std::vector<RiaQDateTimeTools::DateTimePeriod> dateTimePeriods();
    static QString dateTimePeriodName( RiaQDateTimeTools::DateTimePeriod period );

    // This function uses C locale to make sure the text representation of a date is stable, independent of the locale
    // settings on local machine. Required for stable regression testing.
    static QString toStringUsingApplicationLocale( const QDateTime& dt, const QString& format );

    static QString createTimeFormatStringFromDates( const std::vector<QDateTime>& dates );
    static QString dateFormatString();

    static std::vector<QString> supportedDateFormats();
    static std::vector<QString> supportedTimeFormats();

    static QString
        dateFormatString( const QString&       fullDateFormat,
                          DateFormatComponents dateComponents = DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY );
    static QString
        timeFormatString( const QString&       fullTimeFormat,
                          TimeFormatComponents timeComponents = TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE_SECOND );

    static QList<caf::PdmOptionItemInfo> createOptionItems( const std::vector<time_t>& timeSteps );

private:
    static quint64 secondsInDay();
    static quint64 secondsInYear();
};

//==================================================================================================
///
//==================================================================================================
class DateTimeSpan
{
public:
    DateTimeSpan()
        : m_years( 0 )
        , m_months( 0 )
        , m_days( 0 )
    {
    }
    DateTimeSpan( int years, int months, int days )
        : m_years( years )
        , m_months( months )
        , m_days( days )
    {
    }

    int years() const { return m_years; }
    int months() const { return m_months; }
    int days() const { return m_days; }

    bool isEmpty() { return m_years == 0 && m_months == 0 && m_days; }

private:
    int m_years;
    int m_months;
    int m_days;
};
