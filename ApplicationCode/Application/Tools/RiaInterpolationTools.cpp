/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020     Equinor ASA
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

#include "RiaInterpolationTools.h"

#include <cassert>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaInterpolationTools::linear( const std::vector<double>& x, const std::vector<double>& y, double value )
{
    assert( x.size() == y.size() );

    // Handle cases with only one data point.
    if ( x.size() == 1 )
    {
        return std::numeric_limits<double>::infinity();
    }

    // Find the lower boundary
    bool found      = false;
    int  lowerIndex = 0;
    for ( int i = 0; i < static_cast<int>( x.size() - 1 ); i++ )
    {
        if ( x[i] < value && x[i + 1] > value )
        {
            lowerIndex = i;
            found      = true;
        }
    }

    // Value is outside of the defined range
    if ( !found )
    {
        return std::numeric_limits<double>::infinity();
    }

    int upperIndex = lowerIndex + 1;

    double lowerX = x[lowerIndex];
    double lowerY = y[lowerIndex];
    double upperX = x[upperIndex];
    double upperY = y[upperIndex];

    double deltaY = upperY - lowerY;
    double deltaX = upperX - lowerX;

    return lowerY + ( ( value - lowerX ) / deltaX ) * deltaY;
}
