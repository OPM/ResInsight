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

#include <string>

class QDateTime;
class QDate;
class QTime;
class DateTimeSpan;

//==================================================================================================
// 
//==================================================================================================
enum class DateTimePeriod
{
    DECADE,
    YEAR,
    MONTH,
    DAY
};

//==================================================================================================
// 
//==================================================================================================
class RiaQDateTimeTools
{
    static const DateTimeSpan TIMESPAN_DECADE;
    static const DateTimeSpan TIMESPAN_YEAR;
    static const DateTimeSpan TIMESPAN_MONTH;
    static const DateTimeSpan TIMESPAN_DAY;

public:
    static Qt::TimeSpec currentTimeSpec();

    static QDateTime fromString(const QString& dateString, const QString& format);
    static QDateTime fromYears(double years);
    static QDateTime fromTime_t(time_t t);

    static QDateTime addMSecs(const QDateTime& dt, double msecs);
    static QDateTime addDays(const QDateTime& dt, double days);
    static QDateTime addYears(const QDateTime& dt, double years);
    static QDateTime addSpan(const QDateTime& dt, DateTimeSpan span);
    static QDateTime addPeriod(const QDateTime& dt, DateTimePeriod period);

    static QDateTime epoch();

    static QDateTime createUtcDateTime();
    static QDateTime createUtcDateTime(const QDate& date);
    static QDateTime createUtcDateTime(const QDate& date, const QTime& time);
    static QDateTime createUtcDateTime(const QDateTime& dt);

    static bool      equalTo(const QDateTime& dt1, const QDateTime& dt2);
    static bool      lessThan(const QDateTime& dt1, const QDateTime& dt2);
    static bool      lessThanOrEqualTo(const QDateTime& dt1, const QDateTime& dt2);
    static bool      biggerThan(const QDateTime& dt1, const QDateTime& dt2);
    static bool      biggerThanOrEqualTo(const QDateTime& dt1, const QDateTime& dt2);

    static const DateTimeSpan   timeSpan(DateTimePeriod period);
    static QDateTime            truncateTime(const QDateTime& dt, DateTimePeriod period);

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
