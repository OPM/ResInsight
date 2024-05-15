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

#include "RiaDateTimeDefines.h"

#include "cafAppEnum.h"

#include <QString>

#include <set>
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
public:
    static Qt::TimeSpec currentTimeSpec();

    static QDateTime fromString( const QString& dateString, const QString& format );
    static QDateTime fromYears( double years );
    static QDateTime fromTime_t( time_t t );

    static QDateTime addMSecs( const QDateTime& dt, double msecs );
    static QDateTime addDays( const QDateTime& dt, double days );
    static QDateTime addYears( const QDateTime& dt, double years );
    static QDateTime addSpan( const QDateTime& dt, DateTimeSpan span );
    static QDateTime subtractSpan( const QDateTime& dt, DateTimeSpan span );
    static QDateTime addPeriod( const QDateTime& dt, RiaDefines::DateTimePeriod period );
    static QDateTime subtractPeriod( const QDateTime& dt, RiaDefines::DateTimePeriod period );

    static QDateTime createDateTime( const QDate& date, Qt::TimeSpec timeSpec = Qt::LocalTime );

    static QDateTime epoch();

    static QDateTime createUtcDateTime();
    static QDateTime createUtcDateTime( const QDate& date );
    static QDateTime createUtcDateTime( const QDate& date, const QTime& time );
    static QDateTime createUtcDateTime( const QDateTime& dt );

    static bool lessThan( const QDateTime& dt1, const QDateTime& dt2 );

    static const DateTimeSpan timeSpan( RiaDefines::DateTimePeriod period );
    static QDateTime          truncateTime( const QDateTime& dt, RiaDefines::DateTimePeriod period );

    static std::vector<RiaDefines::DateTimePeriod> dateTimePeriods();
    static QString                                 dateTimePeriodName( RiaDefines::DateTimePeriod period );

    // This function uses C locale to make sure the text representation of a date is stable, independent of the locale
    // settings on local machine. Required for stable regression testing.
    static QString toStringUsingApplicationLocale( const QDateTime& dt, const QString& format );

    static QString createTimeFormatStringFromDates( const std::vector<QDateTime>& dates );
    static QString dateFormatString();

    static std::vector<QString> supportedDateFormats();
    static std::vector<QString> supportedTimeFormats();

    static QString
        dateFormatString( const QString&                   fullDateFormat,
                          RiaDefines::DateFormatComponents dateComponents = RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY );
    static QString
        timeFormatString( const QString& fullTimeFormat,
                          RiaDefines::TimeFormatComponents timeComponents = RiaDefines::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE_SECOND );

    static QList<caf::PdmOptionItemInfo> createOptionItems( const std::vector<time_t>& timeSteps );

    static std::set<QDateTime> createEvenlyDistributedDates( const std::vector<QDateTime>& inputDates, int numDates );
    static std::set<QDateTime>
        createEvenlyDistributedDatesInInterval( const QDateTime& fromTimeStep, const QDateTime& toTimeStep, int numDates );
    static std::vector<QDateTime>
        getTimeStepsWithinSelectedRange( const std::vector<QDateTime>& timeSteps, const QDateTime& fromTimeStep, const QDateTime& toTimeStep );

private:
    static const DateTimeSpan TIMESPAN_MINUTE;
    static const DateTimeSpan TIMESPAN_HOUR;
    static const DateTimeSpan TIMESPAN_DAY;
    static const DateTimeSpan TIMESPAN_WEEK;
    static const DateTimeSpan TIMESPAN_MONTH;
    static const DateTimeSpan TIMESPAN_QUARTER;
    static const DateTimeSpan TIMESPAN_HALFYEAR;
    static const DateTimeSpan TIMESPAN_YEAR;
    static const DateTimeSpan TIMESPAN_DECADE;

    static quint64 secondsInDay();
    static quint64 secondsInYear();
    static quint64 secondsInHour();
    static quint64 secondsInMinute();
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
        , m_hours( 0 )
        , m_minutes( 0 )
    {
    }
    DateTimeSpan( int years, int months, int days, int hours = 0, int minutes = 0 )
        : m_years( years )
        , m_months( months )
        , m_days( days )
        , m_hours( hours )
        , m_minutes( minutes )
    {
    }

    int years() const { return m_years; }
    int months() const { return m_months; }
    int days() const { return m_days; }
    int hours() const { return m_hours; }
    int minutes() const { return m_minutes; }

    bool isEmpty() { return m_years == 0 && m_months == 0 && m_days == 0 && m_hours == 0 && m_minutes == 0; }

private:
    int m_years;
    int m_months;
    int m_days;
    int m_hours;
    int m_minutes;
};
