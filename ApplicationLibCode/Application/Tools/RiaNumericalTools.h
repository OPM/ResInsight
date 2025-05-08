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

#pragma once

#include <algorithm>
#include <utility>

namespace RiaNumericalTools
{
double roundToClosestPowerOfTenCeil( double value );
double roundToClosestPowerOfTenFloor( double value );
double computeTenExponentCeil( double value );
double computeTenExponentFloor( double value );

double roundToNumSignificantDigitsFloor( double value, double numSignificantDigits );
double roundToNumSignificantDigitsCeil( double value, double numSignificantDigits );

enum class RoundToSignificantDigitsMode
{
    CEIL,
    FLOOR
};
double roundToNumSignificantDigits( double value, double numSignificantDigits, RoundToSignificantDigitsMode mode );

template <typename T>
bool isValueInRange( T value, const std::pair<T, T>& range )
{
    auto minimumValue = std::min( range.first, range.second );
    auto maximumValue = std::max( range.first, range.second );
    return value >= minimumValue && value <= maximumValue;
}

}; // namespace RiaNumericalTools
