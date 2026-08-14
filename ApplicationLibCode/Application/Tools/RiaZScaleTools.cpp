/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RiaZScaleTools.h"

#include "RiaDefines.h"

#include <algorithm>
#include <set>

namespace
{
std::set<double>& customScaleFactors()
{
    static std::set<double> factors;
    return factors;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaZScaleTools::registerScaleFactor( double scaleFactor )
{
    if ( scaleFactor <= 0.0 ) return;

    customScaleFactors().insert( scaleFactor );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RiaZScaleTools::scaleFactorOptions()
{
    std::set<double> options = customScaleFactors();

    auto predefinedOptions = RiaDefines::viewScaleOptions();
    options.insert( predefinedOptions.begin(), predefinedOptions.end() );

    return { options.begin(), options.end() };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaZScaleTools::nextScaleFactor( double currentScaleFactor, const std::vector<double>& sortedOptions )
{
    auto it = std::upper_bound( sortedOptions.begin(), sortedOptions.end(), currentScaleFactor );
    if ( it == sortedOptions.end() ) return currentScaleFactor;

    return *it;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaZScaleTools::previousScaleFactor( double currentScaleFactor, const std::vector<double>& sortedOptions )
{
    auto it = std::lower_bound( sortedOptions.begin(), sortedOptions.end(), currentScaleFactor );
    if ( it == sortedOptions.begin() ) return currentScaleFactor;

    return *( it - 1 );
}
