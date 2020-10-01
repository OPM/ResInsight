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

#include "RimObjectiveFunction.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double ObjectiveFunction::stdDeviation() const
{
    double N = static_cast<double>( values.size() );
    if ( N > 1 )
    {
        double sumValues        = 0.0;
        double sumValuesSquared = 0.0;
        for ( const QVariant& variant : values )
        {
            double value = variant.toDouble();
            sumValues += value;
            sumValuesSquared += value * value;
        }

        return std::sqrt( ( N * sumValuesSquared - sumValues * sumValues ) / ( N * ( N - 1.0 ) ) );
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
/// Standard deviation normalized by max absolute value of min/max values.
/// Produces values between 0.0 and sqrt(2.0).
//--------------------------------------------------------------------------------------------------
double ObjectiveFunction::normalizedStdDeviation() const
{
    const double eps = 1.0e-4;

    double maxAbs = std::max( std::fabs( maxValue ), std::fabs( minValue ) );
    if ( maxAbs < eps )
    {
        return 0.0;
    }

    double normalisedStdDev = stdDeviation() / maxAbs;
    if ( normalisedStdDev < eps )
    {
        return 0.0;
    }
    return normalisedStdDev;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ObjectiveFunction::operator<( const ObjectiveFunction& other ) const
{
    return this->name < other.name;
}
