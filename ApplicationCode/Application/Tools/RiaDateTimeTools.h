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

#include <string>

//==================================================================================================
// 
//==================================================================================================
class RiaDateTimeTools
{
public:
    static quint64  secondsInMinute()        { return 60;                   }
    static quint64  secondsInHour()          { return 60 * 60;              }
    static quint64  secondsInDay()           { return 60 * 60 * 24;         }
    static quint64  secondsInYear()          { return 60 * 60 * 24 * 365;   }

    static quint64  secondsFromUnit(const std::string& unit);

    static time_t   createFromSecondsSinceEpoch(quint64 secondsSinceEpoch);

private:
};

