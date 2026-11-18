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

#include "RiaKeyValueStore.h"

#include <cstdint>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

//==================================================================================================
///
//==================================================================================================
class RiaKeyValueStoreUtil
{
public:
    // Element type stored for values of type T
    template <typename T>
    static constexpr RiaKeyValueStoreElementType elementType()
    {
        if constexpr ( std::is_same_v<T, float> )
            return RiaKeyValueStoreElementType::FLOAT32;
        else if constexpr ( std::is_same_v<T, double> )
            return RiaKeyValueStoreElementType::FLOAT64;
        else if constexpr ( std::is_same_v<T, int32_t> )
            return RiaKeyValueStoreElementType::INT32;
        else if constexpr ( std::is_same_v<T, int64_t> )
            return RiaKeyValueStoreElementType::INT64;
        else if constexpr ( std::is_same_v<T, uint32_t> )
            return RiaKeyValueStoreElementType::UINT32;
        else
        {
            static_assert( std::is_same_v<T, uint64_t>, "Unsupported key-value store element type" );
            return RiaKeyValueStoreElementType::UINT64;
        }
    }

    // Values for a key. Empty if the key is missing or holds values of another element type.
    template <typename T>
    static std::vector<T> getValues( const RiaKeyValueStore<char>& store, const std::string& key )
    {
        return convertToVector<T>( store.get( key, elementType<T>() ) );
    }

    // Store values for a key with the element type matching T
    template <typename T>
    static void setValues( RiaKeyValueStore<char>& store, const std::string& key, const std::vector<T>& values )
    {
        store.set( key, convertToByteVector( values ), elementType<T>() );
    }

    // Instantiated for float, double, int32_t, int64_t, uint32_t and uint64_t
    template <typename T>
    static std::vector<T> convertToVector( const std::optional<std::vector<char>>& input );
    template <typename T>
    static std::vector<char> convertToByteVector( const std::vector<T>& values );
};
