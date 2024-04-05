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

#pragma once

#include <vector>

//==================================================================================================
//
//==================================================================================================
class RiaInterpolationTools
{
public:
    enum class ExtrapolationMode
    {
        NONE,
        CLOSEST,
        TREND
    };

    static double linear( const std::vector<double>& x,
                          const std::vector<double>& y,
                          double                     value,
                          ExtrapolationMode          extrapolationMode = ExtrapolationMode::NONE );

    // Interpolate/extrapolate away inf values in y vector.
    static void interpolateMissingValues( const std::vector<double>& x, std::vector<double>& y );

private:
    static int interpolateRange( int start, int end, int firstPoint, int lastPoint, const std::vector<double>& x, std::vector<double>& y );
    static int extrapolateRange( int start, int end, int firstPoint, int lastPoint, const std::vector<double>& x, std::vector<double>& y );
    static int findNextDataPoint( const std::vector<double>& values, int index );
    static int findPreviousDataPoint( const std::vector<double>& values, int index );
    static double extrapolate( const std::vector<double>& x, const std::vector<double>& y, double value );
    static double extrapolate( double x0, double y0, double x1, double y1, double value );

    static double extrapolateClosestValue( const std::vector<double>& x, const std::vector<double>& y, double value );
};
