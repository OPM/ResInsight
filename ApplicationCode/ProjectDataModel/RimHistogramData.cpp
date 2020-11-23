/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RimHistogramData.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramData::RimHistogramData()
    : min( HUGE_VAL )
    , max( HUGE_VAL )
    , p10( HUGE_VAL )
    , p90( HUGE_VAL )
    , mean( HUGE_VAL )
    , weightedMean( HUGE_VAL )
    , sum( 0.0 )
    , histogram( nullptr )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimHistogramData::isMinMaxValid() const
{
    return isValid( min ) && isValid( max );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimHistogramData::isValid( double parameter ) const
{
    return parameter != HUGE_VAL && parameter != -HUGE_VAL;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimHistogramData::isHistogramVectorValid() const
{
    return histogram && histogram->size() > 0 && isMinMaxValid();
}
