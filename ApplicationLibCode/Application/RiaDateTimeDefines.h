/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022-     Equinor ASA
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

namespace RiaDefines
{
enum class DateFormatComponents
{
    DATE_FORMAT_UNSPECIFIED = -2,
    DATE_FORMAT_NONE        = -1,
    DATE_FORMAT_YEAR        = 0,
    DATE_FORMAT_YEAR_MONTH,
    DATE_FORMAT_YEAR_MONTH_DAY,
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
    MINUTE,
    HOUR,
    DAY,
    WEEK,
    MONTH,
    QUARTER,
    HALFYEAR,
    YEAR,
    DECADE
};

using DateTimePeriodEnum = caf::AppEnum<RiaDefines::DateTimePeriod>;

}; // namespace RiaDefines
