/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include <cmath>
#include <limits>
#include <vector>

//==================================================================================================
//
//
//
//==================================================================================================
class RigStatisticsTools
{
public:
    template <class NumberType>
    static bool isInvalidNumber( NumberType value )
    {
        return !isValidNumber<NumberType>( value );
    }

    template <class NumberType>
    static bool isValidNumber( NumberType value )
    {
        return !std::isinf( value ) && !std::isnan( value );
    }

    template <class NumberType>
    static NumberType minimumValue( const std::vector<NumberType>& values )
    {
        NumberType minValue = std::numeric_limits<NumberType>::max();
        for ( NumberType value : values )
        {
            if ( RigStatisticsTools::isValidNumber<NumberType>( value ) )
            {
                minValue = std::min( minValue, value );
            }
        }

        return minValue;
    }

    template <class NumberType>
    static NumberType maximumValue( const std::vector<NumberType>& values )
    {
        NumberType maxValue = -std::numeric_limits<NumberType>::max();
        for ( NumberType value : values )
        {
            if ( RigStatisticsTools::isValidNumber<NumberType>( value ) )
            {
                maxValue = std::max( maxValue, value );
            }
        }

        return maxValue;
    }

    static double pearsonCorrelation( const std::vector<double>& xValues, const std::vector<double>& yValues );
};
