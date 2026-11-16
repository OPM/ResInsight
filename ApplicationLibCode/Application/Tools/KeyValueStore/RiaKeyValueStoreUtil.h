/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include <optional>
#include <vector>

//==================================================================================================
///
//==================================================================================================
class RiaKeyValueStoreUtil
{
public:
    static std::vector<float> convertToFloatVector( const std::optional<std::vector<char>>& input );

    // Instantiated for float, double, int32_t, int64_t, uint32_t and uint64_t
    template <typename T>
    static std::vector<T> convertToVector( const std::optional<std::vector<char>>& input );
    template <typename T>
    static std::vector<char> convertToByteVector( const std::vector<T>& values );
};
