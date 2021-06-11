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

#include "RigHistogramData.h"

#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigHistogramData::RigHistogramData()
    : min( std::numeric_limits<double>::infinity() )
    , max( std::numeric_limits<double>::infinity() )
    , p10( std::numeric_limits<double>::infinity() )
    , p90( std::numeric_limits<double>::infinity() )
    , mean( std::numeric_limits<double>::infinity() )
    , weightedMean( std::numeric_limits<double>::infinity() )
    , sum( 0.0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigHistogramData::isMinMaxValid() const
{
    return isValid( min ) && isValid( max );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigHistogramData::isValid( double parameter ) const
{
    return parameter != std::numeric_limits<double>::infinity() && parameter != -std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigHistogramData::isHistogramVectorValid() const
{
    return histogram.size() > 0 && isMinMaxValid();
}
