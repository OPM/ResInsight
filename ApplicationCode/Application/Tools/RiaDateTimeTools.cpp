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

#include "RiaDateTimeTools.h"

#include <ctime>

#include <QString>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
quint64 RiaDateTimeTools::secondsFromUnit(const std::string& unit)
{
    QString str = QString::fromStdString(unit).trimmed().toUpper();

    if (str == "DAYS")
    {
        return RiaDateTimeTools::secondsInDay();
    }
    else if (str == "YEARS")
    {
        return RiaDateTimeTools::secondsInYear();
    }

    return 1;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
time_t RiaDateTimeTools::createFromSecondsSinceEpoch(quint64 secondsSinceEpoch)
{
    return time_t(secondsSinceEpoch);
}
