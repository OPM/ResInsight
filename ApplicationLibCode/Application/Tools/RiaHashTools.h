/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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

#include <cstddef>
#include <functional>
#include <ranges>
#include <string>
#include <type_traits>

//==================================================================================================
//
//
//
//==================================================================================================
namespace RiaHashTools
{
//--------------------------------------------------------------------------------------------------
/// Variadic template function to combine multiple parameters into a single hash
/// Constrained to non-range types (std::string is excluded as it has std::hash specialization)
//--------------------------------------------------------------------------------------------------
template <typename T>
    requires( !std::ranges::range<T> || std::is_same_v<std::decay_t<T>, std::string> )
void combineHash( size_t& seed, const T& value )
{
    // Based on https://www.boost.org/doc/libs/1_84_0/libs/container_hash/doc/html/hash.html#notes_hash_combine
    seed ^= std::hash<T>()( value ) + 0x9e3779b9 + ( seed << 6 ) + ( seed >> 2 );
}

//--------------------------------------------------------------------------------------------------
/// Overload for range types (excluding std::string)
//--------------------------------------------------------------------------------------------------
template <std::ranges::range Range>
    requires( !std::is_same_v<std::decay_t<Range>, std::string> )
void combineHash( size_t& seed, const Range& range )
{
    for ( const auto& elem : range )
    {
        combineHash( seed, elem );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T, typename... Rest>
void combineHash( size_t& seed, const T& value, const Rest&... rest )
{
    combineHash( seed, value );
    ( combineHash( seed, rest ), ... );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <std::ranges::range Range, typename... Rest>
void combineHash( size_t& seed, const Range& range, const Rest&... rest )
{
    for ( const auto& elem : range )
    {
        combineHash( seed, elem );
    }
    ( combineHash( seed, rest ), ... );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename... Args>
size_t hash( const Args&... args )
{
    size_t seed = 0;
    combineHash( seed, args... );
    return seed;
}

//--------------------------------------------------------------------------------------------------
/// Generic hash function for any range
//--------------------------------------------------------------------------------------------------
template <std::ranges::range Range>
size_t hash( const Range& range )
{
    size_t seed = 0;
    for ( const auto& elem : range )
    {
        combineHash( seed, elem );
    }
    return seed;
}

}; // namespace RiaHashTools
