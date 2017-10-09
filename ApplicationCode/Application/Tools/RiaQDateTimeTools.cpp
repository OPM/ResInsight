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

#include <QString>
#include <QDateTime>

#include <ctime>
#include <cmath>


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
QDateTime RiaQDateTimeTools::createDateTime()
{
    auto& qdt = QDateTime();
    qdt.setTimeSpec(currentTimeSpec());
    return qdt;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime RiaQDateTimeTools::createDateTime(const QDate& date)
{
    auto& qdt = QDateTime(date);
    qdt.setTimeSpec(currentTimeSpec());
    return qdt;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime RiaQDateTimeTools::createDateTime(const QDate& date, const QTime& time)
{
    auto& qdt = QDateTime(date, time, currentTimeSpec());
    return qdt;
}
