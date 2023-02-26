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

#include "RiaDateTimeDefines.h"

#include "cafAppEnum.h"

namespace caf
{
template <>
void caf::AppEnum<RiaDefines::DateFormatComponents>::setUp()
{
    addItem( RiaDefines::DateFormatComponents::DATE_FORMAT_NONE, "NO_DATE", "No Date" );
    addItem( RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR, "YEAR", "Year Only" );
    addItem( RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH, "YEAR_MONTH", "Year and Month" );
    addItem( RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY, "YEAR_MONTH_DAY", "Year, Month and Day" );
    setDefault( RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY );
}

template <>
void caf::AppEnum<RiaDefines::TimeFormatComponents>::setUp()
{
    addItem( RiaDefines::TimeFormatComponents::TIME_FORMAT_NONE, "NO_TIME", "No Time of Day" );
    addItem( RiaDefines::TimeFormatComponents::TIME_FORMAT_HOUR, "HOUR", "Hour Only" );
    addItem( RiaDefines::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE, "HOUR_MINUTE", "Hour and Minute" );
    addItem( RiaDefines::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE_SECOND, "HOUR_MINUTE_SECONDS", "Hour, Minutes and Seconds" );
    setDefault( RiaDefines::TimeFormatComponents::TIME_FORMAT_NONE );
}

template <>
void caf::AppEnum<RiaDefines::DateTimePeriod>::setUp()
{
    addItem( RiaDefines::DateTimePeriod::NONE, "NONE", "None" );
    addItem( RiaDefines::DateTimePeriod::DAY, "DAY", "Day" );
    addItem( RiaDefines::DateTimePeriod::WEEK, "WEEK", "Week" );
    addItem( RiaDefines::DateTimePeriod::MONTH, "MONTH", "Month" );
    addItem( RiaDefines::DateTimePeriod::QUARTER, "QUARTER", "Quarter" );
    addItem( RiaDefines::DateTimePeriod::HALFYEAR, "HALFYEAR", "Half Year" );
    addItem( RiaDefines::DateTimePeriod::YEAR, "YEAR", "Year" );
    addItem( RiaDefines::DateTimePeriod::DECADE, "DECADE", "Decade" );
    setDefault( RiaDefines::DateTimePeriod::NONE );
}
} // namespace caf
