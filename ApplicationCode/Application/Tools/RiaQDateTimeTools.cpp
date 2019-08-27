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

#include "RiaQDateTimeTools.h"

#include <QDateTime>
#include <QLocale>
#include <QString>

#include "cafAppEnum.h"
#include <cvfAssert.h>

#include <ctime>
#include <cmath>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const DateTimeSpan RiaQDateTimeTools::TIMESPAN_DAY = DateTimeSpan(0, 0, 1);
const DateTimeSpan RiaQDateTimeTools::TIMESPAN_WEEK = DateTimeSpan(0, 0, 7);
const DateTimeSpan RiaQDateTimeTools::TIMESPAN_MONTH = DateTimeSpan(0, 1, 0);
const DateTimeSpan RiaQDateTimeTools::TIMESPAN_QUARTER = DateTimeSpan(0, 3, 0);
const DateTimeSpan RiaQDateTimeTools::TIMESPAN_HALFYEAR = DateTimeSpan(0, 6, 0);
const DateTimeSpan RiaQDateTimeTools::TIMESPAN_YEAR = DateTimeSpan(1, 0, 0);
const DateTimeSpan RiaQDateTimeTools::TIMESPAN_DECADE = DateTimeSpan(10, 0, 0);

const QString RiaQDateTimeTools::TIMESPAN_DAY_NAME = "Day";
const QString RiaQDateTimeTools::TIMESPAN_WEEK_NAME = "Week";
const QString RiaQDateTimeTools::TIMESPAN_MONTH_NAME = "Month";
const QString RiaQDateTimeTools::TIMESPAN_QUARTER_NAME = "Quarter";
const QString RiaQDateTimeTools::TIMESPAN_HALFYEAR_NAME = "Half Year";
const QString RiaQDateTimeTools::TIMESPAN_YEAR_NAME = "Year";
const QString RiaQDateTimeTools::TIMESPAN_DECADE_NAME = "Decade";


namespace caf
{
template<>
void caf::AppEnum<RiaQDateTimeTools::DateFormatComponents>::setUp()
{
	addItem(RiaQDateTimeTools::DATE_FORMAT_NONE, "NO_DATE", "No Date");
	addItem(RiaQDateTimeTools::DATE_FORMAT_YEAR, "YEAR", "Year Only");
	addItem(RiaQDateTimeTools::DATE_FORMAT_YEAR_MONTH, "YEAR_MONTH", "Year and Month");
	addItem(RiaQDateTimeTools::DATE_FORMAT_YEAR_MONTH_DAY, "YEAR_MONTH_DAY", "Year, Month and Day");
	setDefault(RiaQDateTimeTools::DATE_FORMAT_YEAR_MONTH_DAY);
}

template<>
void caf::AppEnum<RiaQDateTimeTools::TimeFormatComponents>::setUp()
{
    addItem(RiaQDateTimeTools::TIME_FORMAT_NONE, "NO_TIME", "No Time of Day");
    addItem(RiaQDateTimeTools::TIME_FORMAT_HOUR, "HOUR", "Hour Only");
    addItem(RiaQDateTimeTools::TIME_FORMAT_HOUR_MINUTE, "HOUR_MINUTE", "Hour and Minute");
    addItem(RiaQDateTimeTools::TIME_FORMAT_HOUR_MINUTE_SECOND, "HOUR_MINUTE_SECONDS", "Hour, Minutes and Seconds");
	setDefault(RiaQDateTimeTools::TIME_FORMAT_NONE);
}
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Qt::TimeSpec RiaQDateTimeTools::currentTimeSpec()
{
    return Qt::UTC;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
quint64 RiaQDateTimeTools::secondsInDay()
{
    return 60 * 60 * 24;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
quint64 RiaQDateTimeTools::secondsInYear()
{
    return 60 * 60 * 24 * 365;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime RiaQDateTimeTools::fromString(const QString& dateString, const QString& format)
{
    QDateTime dt = QDateTime::fromString(dateString, format);
    dt.setTimeSpec(currentTimeSpec());

    return dt;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime RiaQDateTimeTools::fromYears(double years)
{
    double yearsAfterEpoch = years - 1970.0;

    QDateTime dt = RiaQDateTimeTools::epoch();

    return RiaQDateTimeTools::addYears(dt, yearsAfterEpoch);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime RiaQDateTimeTools::fromTime_t(time_t t)
{
    auto qdt = createUtcDateTime();
    qdt.setTime_t(t);
    return qdt;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime RiaQDateTimeTools::addMSecs(const QDateTime& dt, double msecs)
{
    return dt.addMSecs(msecs);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime RiaQDateTimeTools::addDays(const QDateTime& dt, double days)
{
    double integerPart = 0.0;
    double fractionPart = 0.0;

    fractionPart = modf(days, &integerPart);

    QDateTime tmp = dt.addDays(integerPart);
    tmp = tmp.addSecs(fractionPart * RiaQDateTimeTools::secondsInDay());

    return tmp;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime RiaQDateTimeTools::addYears(const QDateTime& dt, double years)
{
    double integerPart = 0.0;
    double fractionPart = 0.0;

    fractionPart = modf(years, &integerPart);

    QDateTime tmp = dt.addYears(integerPart);
    tmp = tmp.addSecs(fractionPart * RiaQDateTimeTools::secondsInYear());

    return tmp;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime RiaQDateTimeTools::addSpan(const QDateTime& dt, DateTimeSpan span)
{
    return createUtcDateTime(dt)
        .addYears(span.years())
        .addMonths(span.months())
        .addDays(span.days());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime RiaQDateTimeTools::subtractSpan(const QDateTime& dt, DateTimeSpan span)
{
    return createUtcDateTime(dt)
        .addYears(-span.years())
        .addMonths(-span.months())
        .addDays(-span.days());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime RiaQDateTimeTools::addPeriod(const QDateTime& dt, DateTimePeriod period)
{
    return addSpan(dt, timeSpan(period));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime RiaQDateTimeTools::subtractPeriod(const QDateTime& dt, DateTimePeriod period)
{
    return subtractSpan(dt, timeSpan(period));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime RiaQDateTimeTools::epoch()
{

    // NB: Not able to use QDateTime::fromMSecsSinceEpoch as this was introduced in Qt 4.7

    QDateTime dt;
    dt.setDate(QDate(1970, 1, 1)); 
    dt.setTimeSpec(currentTimeSpec());

    return dt;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime RiaQDateTimeTools::createUtcDateTime()
{
    auto qdt = QDateTime();
    qdt.setTimeSpec(currentTimeSpec());
    return qdt;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime RiaQDateTimeTools::createUtcDateTime(const QDate& date)
{
    auto qdt = QDateTime(date);
    qdt.setTimeSpec(currentTimeSpec());
    return qdt;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime RiaQDateTimeTools::createUtcDateTime(const QDate& date, const QTime& time)
{
    auto qdt = QDateTime(date, time, currentTimeSpec());
    return qdt;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime RiaQDateTimeTools::createUtcDateTime(const QDateTime& dt)
{
    auto qdt = QDateTime(dt);
    qdt.setTimeSpec(currentTimeSpec());
    return qdt;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaQDateTimeTools::lessThan(const QDateTime& dt1, const QDateTime& dt2)
{
    // dt1 < dt2
    return dt1.secsTo(dt2) > 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const DateTimeSpan RiaQDateTimeTools::timeSpan(DateTimePeriod period)
{
    switch (period)
    {
    case DateTimePeriod::DAY:       return TIMESPAN_DAY;
    case DateTimePeriod::WEEK:      return TIMESPAN_WEEK;
    case DateTimePeriod::MONTH:     return TIMESPAN_MONTH;
    case DateTimePeriod::QUARTER:   return TIMESPAN_QUARTER;
    case DateTimePeriod::HALFYEAR:  return TIMESPAN_HALFYEAR;
    case DateTimePeriod::YEAR:      return TIMESPAN_YEAR;
    case DateTimePeriod::DECADE:    return TIMESPAN_DECADE;
    }
    CVF_ASSERT(false);
    return DateTimeSpan();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime RiaQDateTimeTools::truncateTime(const QDateTime& dt, DateTimePeriod period)
{
    int y = dt.date().year();
    int m = dt.date().month();
    int d = dt.date().day();
    int dow = dt.date().dayOfWeek();

    switch (period)
    {
    case DateTimePeriod::DAY:       return createUtcDateTime(QDate(y, m, d));
    case DateTimePeriod::WEEK:      return createUtcDateTime(QDate(y, m, d).addDays(-dow + 1));
    case DateTimePeriod::MONTH:     return createUtcDateTime(QDate(y, m, 1));
    case DateTimePeriod::QUARTER:   return createUtcDateTime(QDate(y, ((m - 1) / 3) * 3 + 1, 1));
    case DateTimePeriod::HALFYEAR:  return createUtcDateTime(QDate(y, ((m - 1) / 6) * 6 + 1, 1));
    case DateTimePeriod::YEAR:      return createUtcDateTime(QDate(y, 1, 1));
    case DateTimePeriod::DECADE:    return createUtcDateTime(QDate((y / 10) * 10, 1, 1));
    }
    CVF_ASSERT(false);
    return createUtcDateTime();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<DateTimePeriod> RiaQDateTimeTools::dateTimePeriods()
{
    return std::vector<DateTimePeriod>(
        {
            DateTimePeriod::NONE,
            DateTimePeriod::DAY,
            DateTimePeriod::WEEK,
            DateTimePeriod::MONTH,
            DateTimePeriod::QUARTER,
            DateTimePeriod::HALFYEAR,
            DateTimePeriod::YEAR,
            DateTimePeriod::DECADE,
        });
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaQDateTimeTools::dateTimePeriodName(DateTimePeriod period)
{
    switch (period)
    {
    case DateTimePeriod::DAY:       return TIMESPAN_DAY_NAME;
    case DateTimePeriod::WEEK:      return TIMESPAN_WEEK_NAME;
    case DateTimePeriod::MONTH:     return TIMESPAN_MONTH_NAME;
    case DateTimePeriod::QUARTER:   return TIMESPAN_QUARTER_NAME;
    case DateTimePeriod::HALFYEAR:  return TIMESPAN_HALFYEAR_NAME;
    case DateTimePeriod::YEAR:      return TIMESPAN_YEAR_NAME;
    case DateTimePeriod::DECADE:    return TIMESPAN_DECADE_NAME;
    default:                        return "None";
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaQDateTimeTools::toStringUsingApplicationLocale(const QDateTime& dt, const QString& format)
{
    // Default application locale is set in RiaMain
    // QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    //
    // QDate/QDateTime use system locale for toString() functions

    QLocale defaultApplicationLocale;

    return defaultApplicationLocale.toString(dt, format);
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaQDateTimeTools::createTimeFormatStringFromDates(const std::vector<QDateTime>& dates)
{
    bool hasHoursAndMinutesInTimesteps = false;
    bool hasSecondsInTimesteps         = false;
    bool hasMillisecondsInTimesteps    = false;

    for (size_t i = 0; i < dates.size(); i++)
    {
        if (dates[i].time().msec() != 0.0)
        {
            hasMillisecondsInTimesteps    = true;
            hasSecondsInTimesteps         = true;
            hasHoursAndMinutesInTimesteps = true;
            break;
        }
        else if (dates[i].time().second() != 0.0)
        {
            hasHoursAndMinutesInTimesteps = true;
            hasSecondsInTimesteps         = true;
        }
        else if (dates[i].time().hour() != 0.0 || dates[i].time().minute() != 0.0)
        {
            hasHoursAndMinutesInTimesteps = true;
        }
    }

    QString formatString = dateFormatString();
    if (hasHoursAndMinutesInTimesteps)
    {
        formatString += " - hh:mm";
        if (hasSecondsInTimesteps)
        {
            formatString += ":ss";
            if (hasMillisecondsInTimesteps)
            {
                formatString += ".zzz";
            }
        }
    }

    return formatString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaQDateTimeTools::dateFormatString()
{
    return "dd.MMM yyyy";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RiaQDateTimeTools::supportedDateFormats()
{
    std::vector<QString> dateFormats;

    // See enum DateFormatComponents in header
    // The semi-colon separated components are:
    // DATE_FORMAT_YEAR, ..YEAR_MONTH, ..YEAR_MONTH_DAY
    dateFormats.push_back("yyyy;yyyy-MM;yyyy-MM-dd");
    dateFormats.push_back("yyyy;MMM yyyy;dd. MMM yyyy");
    dateFormats.push_back("yyyy;MMM yyyy;MMM dd. yyyy");
    dateFormats.push_back("yyyy;MM/yyyy;dd/MM/yyyy");
    dateFormats.push_back("yyyy;M/yyyy;d/M/yyyy");
    dateFormats.push_back("yyyy;M/yyyy;M/d/yyyy");
    dateFormats.push_back("yy;M/yy;d/M/yy");
    dateFormats.push_back("yy;M/yy;M/d/yy");
    dateFormats.push_back("yyyy;MM-yyyy;dd-MM-yyyy");
    dateFormats.push_back("yyyy;MM.yyyy;dd.MM.yyyy");
    dateFormats.push_back("yyyy;MM-yyyy;MM-dd-yyyy");
    dateFormats.push_back("yyyy;MM.yyyy;MM.dd.yyyy");
    dateFormats.push_back("yy;MM-yy;dd-MM-yy");
    dateFormats.push_back("yy;MM-yy;MM-dd-yy");

    return dateFormats;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RiaQDateTimeTools::supportedTimeFormats()
{
    std::vector<QString> timeFormats;

    // See enum TimeFormatComponents in header
    // The semi-colon separated components are:
    // TIME_FORMAT_HOUR, ..HOUR_MINUTE, ..HOUR_MINUTE_SECOND and ..HOUR_MINUTE_MILLISECOND
    timeFormats.push_back("HH;HH:mm;HH:mm:ss;HH:mm:ss.zzz");
    timeFormats.push_back("H;H:mm;H:mm:ss;H:mm:ss.zzz");
    timeFormats.push_back("hh AP;hh:mm AP;hh:mm:ss AP;hh:mm:ss.zzz AP");
    timeFormats.push_back("h AP;h:mm AP;h:mm:ss AP;h:mm:ss.zzz AP");

    return timeFormats;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaQDateTimeTools::dateFormatString(const QString& fullDateFormat, DateFormatComponents dateComponents)
{
    if (dateComponents == DATE_FORMAT_NONE) return "";

    QStringList allVariants = fullDateFormat.split(";");
    if (static_cast<int>(dateComponents) < allVariants.size())
    {
        return allVariants[dateComponents];
    }
    CVF_ASSERT(false && "Date format string is malformed");
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaQDateTimeTools::timeFormatString(const QString& fullTimeFormat, TimeFormatComponents timeComponents)
{
    if (timeComponents == TIME_FORMAT_NONE) return "";

    QStringList allVariants = fullTimeFormat.split(";");
    if (static_cast<int>(timeComponents) < allVariants.size())
    {
        return allVariants[timeComponents];
    }
    CVF_ASSERT(false && "Time format string is malformed");
    return "";
}