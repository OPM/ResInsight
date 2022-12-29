/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022 Equinor ASA
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

#include "RiaNumericalTools.h"

#include "cvfMath.h"

#include <algorithm>
#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaNumericalTools::roundToClosestPowerOfTenCeil( double value )
{
    auto exponent = computeTenExponentCeil( value );
    return pow( 10.0, exponent );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaNumericalTools::roundToClosestPowerOfTenFloor( double value )
{
    auto exponent = computeTenExponentFloor( value );
    return pow( 10.0, exponent );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaNumericalTools::computeTenExponentCeil( double value )
{
    if ( value < 0.0 ) return 0.0;

    double logDecValueMax = log10( value );
    logDecValueMax        = cvf::Math::ceil( logDecValueMax );

    return logDecValueMax;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaNumericalTools::computeTenExponentFloor( double value )
{
    if ( value < 0.0 ) return 0.0;

    double logDecValueMin = log10( value );
    logDecValueMin        = cvf::Math::floor( logDecValueMin );

    return logDecValueMin;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaNumericalTools::roundToNumSignificantDigits( double value, double numSignificantDigits )
{
    double absoluteValue = cvf::Math::abs( value );
    if ( absoluteValue == 0.0 )
    {
        return 0.0;
    }

    double logDecValue = log10( absoluteValue );
    logDecValue        = cvf::Math::ceil( logDecValue );

    double factor = pow( 10.0, numSignificantDigits - logDecValue );

    double tmp = value * factor;
    double integerPart;
    double fraction = modf( tmp, &integerPart );

    if ( cvf::Math::abs( fraction ) >= 0.5 ) ( integerPart >= 0 ) ? integerPart++ : integerPart--;

    double roundedValue = integerPart / factor;

    return roundedValue;
}
