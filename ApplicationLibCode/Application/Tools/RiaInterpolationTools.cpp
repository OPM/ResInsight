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

#include "cafAssert.h"

#include <cmath>
#include <cstdlib>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool almostEqual( double a, double b, double maxRelDiff = std::numeric_limits<double>::epsilon() * 128 )
{
    // Calculate the difference.
    double diff  = std::fabs( a - b );
    double fabsa = std::fabs( a );
    double fabsb = std::fabs( b );
    // Find the largest
    double largest = ( fabsb > fabsa ) ? fabsb : fabsa;
    return ( diff <= largest * maxRelDiff );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaInterpolationTools::linear( const std::vector<double>& x, const std::vector<double>& y, double value, ExtrapolationMode extrapolationMode )
{
    CAF_ASSERT( x.size() == y.size() );

    // Handle cases with only one data point.
    if ( x.size() <= 1 )
    {
        return std::numeric_limits<double>::infinity();
    }

    // Find the lower boundary
    bool found      = false;
    int  lowerIndex = 0;
    for ( int i = 0; i < static_cast<int>( x.size() - 1 ); i++ )
    {
        if ( x[i] <= value && x[i + 1] >= value )
        {
            lowerIndex = i;
            found      = true;
        }
    }

    // Value is outside of the defined range
    if ( !found )
    {
        // Check if we are just outside the boundaries
        if ( almostEqual( value, x[0] ) )
            return y[0];
        else if ( almostEqual( value, x[x.size() - 1] ) )
            return y[x.size() - 1];

        if ( extrapolationMode == ExtrapolationMode::CLOSEST )
        {
            return extrapolateClosestValue( x, y, value );
        }
        else if ( extrapolationMode == ExtrapolationMode::TREND && x.size() >= 2 )
        {
            return extrapolate( x, y, value );
        }

        CAF_ASSERT( extrapolationMode == ExtrapolationMode::NONE );
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaInterpolationTools::extrapolate( const std::vector<double>& x, const std::vector<double>& y, double value )
{
    CAF_ASSERT( x.size() == y.size() );
    CAF_ASSERT( x.size() >= 2 && "Need at least two points for extrapolation" );

    if ( value <= x[0] )
        return extrapolate( x[0], y[0], x[1], y[1], value );
    else
    {
        size_t lastIndex       = x.size() - 1;
        size_t secondLastIndex = x.size() - 2;
        return extrapolate( x[secondLastIndex], y[secondLastIndex], x[lastIndex], y[lastIndex], value );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaInterpolationTools::extrapolate( double x0, double y0, double x1, double y1, double value )
{
    return y0 + ( value - x0 ) / ( x1 - x0 ) * ( y1 - y0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaInterpolationTools::extrapolateClosestValue( const std::vector<double>& x, const std::vector<double>& y, double value )
{
    if ( value <= x[0] )
        return y[0];
    else
        return y[x.size() - 1];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaInterpolationTools::findNextDataPoint( const std::vector<double>& values, int index )
{
    for ( size_t i = index; i < values.size(); i++ )
    {
        if ( values[i] != std::numeric_limits<double>::infinity() ) return static_cast<int>( i );
    }

    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaInterpolationTools::findPreviousDataPoint( const std::vector<double>& values, int index )
{
    CAF_ASSERT( index >= 0 );

    for ( int i = index; i >= 0; i-- )
    {
        if ( values[i] != std::numeric_limits<double>::infinity() ) return static_cast<int>( i );
    }

    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaInterpolationTools::extrapolateRange( int start, int end, int firstPoint, int lastPoint, const std::vector<double>& x, std::vector<double>& y )
{
    std::vector<double> xs = { x[firstPoint], x[lastPoint] };
    std::vector<double> ys = { y[firstPoint], y[lastPoint] };
    for ( int index = start; index < end; index++ )
    {
        // Avoid excessive extrapolation when points are very close
        if ( almostEqual( xs[0], xs[1] ) || almostEqual( ys[0], ys[1] ) )
            y[index] = ys[0];
        else
            y[index] = extrapolate( xs, ys, x[index] );
    }

    return end;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaInterpolationTools::interpolateRange( int start, int end, int firstPoint, int lastPoint, const std::vector<double>& x, std::vector<double>& y )
{
    CAF_ASSERT( start <= end );

    std::vector<double> xs = { x[firstPoint], x[lastPoint] };
    std::vector<double> ys = { y[firstPoint], y[lastPoint] };
    for ( int index = start; index < end; index++ )
    {
        y[index] = RiaInterpolationTools::linear( xs, ys, x[index] );
    }

    return end;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaInterpolationTools::interpolateMissingValues( const std::vector<double>& x, std::vector<double>& y )
{
    CAF_ASSERT( x.size() == y.size() );

    int index = 0;

    // Previous index which is not inf
    int prevSetIndex = -1;

    while ( index < static_cast<int>( y.size() ) )
    {
        // Missing values are inf in the input data
        if ( y[index] == std::numeric_limits<double>::infinity() )
        {
            // Find the next index with a value
            int nextSetIndex = findNextDataPoint( y, index + 1 );

            if ( prevSetIndex == -1 )
            {
                // The first value is inf: need to find next two valid points and extrapolate
                int nextSetIndex2 = findNextDataPoint( y, nextSetIndex + 1 );
                index             = extrapolateRange( index, nextSetIndex, nextSetIndex, nextSetIndex2, x, y );
            }
            else if ( nextSetIndex == -1 )
            {
                // The last value is inf: extrapolate from two last data points
                int prevSetIndex2 = findPreviousDataPoint( y, prevSetIndex - 1 );
                index             = extrapolateRange( index, (int)y.size(), prevSetIndex2, prevSetIndex, x, y );
            }
            else if ( nextSetIndex != static_cast<int>( y.size() ) )
            {
                // The missing values somewhere between non-inf data: interpolate all the values
                index = interpolateRange( index, nextSetIndex, prevSetIndex, nextSetIndex, x, y );
            }
        }
        else
        {
            // Nothing to do for the values which are not missing
            prevSetIndex = index;
            ++index;
        }
    }
}
