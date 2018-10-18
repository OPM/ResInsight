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

#include <qglobal.h>
#include <qnamespace.h>

#include <QString>

#include <string>
#include <vector>

class QDateTime;
class QDate;
class QTime;
class DateTimeSpan;

//==================================================================================================
// 
//==================================================================================================
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
    static const QString TIMESPAN_DAY_NAME;
    static const QString TIMESPAN_WEEK_NAME;
    static const QString TIMESPAN_MONTH_NAME;
    static const QString TIMESPAN_QUARTER_NAME;
    static const QString TIMESPAN_HALFYEAR_NAME;
    static const QString TIMESPAN_YEAR_NAME;
    static const QString TIMESPAN_DECADE_NAME;

    static Qt::TimeSpec currentTimeSpec();

    static QDateTime fromString(const QString& dateString, const QString& format);
    static QDateTime fromYears(double years);
    static QDateTime fromTime_t(time_t t);

    static QDateTime addMSecs(const QDateTime& dt, double msecs);
    static QDateTime addDays(const QDateTime& dt, double days);
    static QDateTime addYears(const QDateTime& dt, double years);
    static QDateTime addSpan(const QDateTime& dt, DateTimeSpan span);
    static QDateTime subtractSpan(const QDateTime& dt, DateTimeSpan span);
    static QDateTime addPeriod(const QDateTime& dt, DateTimePeriod period);
    static QDateTime subtractPeriod(const QDateTime& dt, DateTimePeriod period);

    static QDateTime epoch();

    static QDateTime createUtcDateTime();
    static QDateTime createUtcDateTime(const QDate& date);
    static QDateTime createUtcDateTime(const QDate& date, const QTime& time);
    static QDateTime createUtcDateTime(const QDateTime& dt);

    static bool      lessThan(const QDateTime& dt1, const QDateTime& dt2);

    static const DateTimeSpan   timeSpan(DateTimePeriod period);
    static QDateTime            truncateTime(const QDateTime& dt, DateTimePeriod period);

    static std::vector<DateTimePeriod>  dateTimePeriods();
    static QString                      dateTimePeriodName(DateTimePeriod period);

    // This function uses C locale to make sure the text representation of a date is stable, independent of the locale
    // settings on local machine. Required for stable regression testing.
    static QString toStringUsingApplicationLocale(const QDateTime& dt, const QString& format);

private:
    static quint64  secondsInDay();
    static quint64  secondsInYear();
};

//==================================================================================================
/// 
//==================================================================================================
class DateTimeSpan
{
public:
    DateTimeSpan() : m_years(0), m_months(0), m_days(0) { }
    DateTimeSpan(int years, int months, int days) : m_years(years), m_months(months), m_days(days) { }

    int years() const { return m_years; }
    int months() const { return m_months; }
    int days() const { return m_days; }

    bool isEmpty() { return m_years == 0 && m_months == 0 && m_days; }

private:
    int m_years;
    int m_months;
    int m_days;
};
