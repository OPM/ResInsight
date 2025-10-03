/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

template <typename Tag>
class TypeSafeIndex
{
public:
    // Default constructor
    constexpr TypeSafeIndex() noexcept
        : m_value( 0 )
    {
    }

    // Explicit constructor from size_t (prevents implicit conversion)
    explicit constexpr TypeSafeIndex( size_t value ) noexcept
        : m_value( value )
    {
    }

    // Explicit conversion to size_t
    explicit constexpr operator size_t() const noexcept { return m_value; }

    // Getter for when you need the value
    constexpr size_t value() const noexcept { return m_value; }

    // Comparison operators
    constexpr bool operator==( const TypeSafeIndex& other ) const noexcept  = default;
    constexpr auto operator<=>( const TypeSafeIndex& other ) const noexcept = default;

    // Arithmetic operators (if needed)
    constexpr TypeSafeIndex& operator++() noexcept
    {
        ++m_value;
        return *this;
    }

private:
    size_t m_value;
};

// Define specific types using tag structs
struct ReservoirCellIndexTag
{
};

using ReservoirCellIndex = TypeSafeIndex<ReservoirCellIndexTag>;

// Hash support for using in std::unordered_map, etc.
namespace std
{
template <typename Tag>
struct hash<TypeSafeIndex<Tag>>
{
    size_t operator()( const TypeSafeIndex<Tag>& idx ) const noexcept { return hash<size_t>{}( idx.value() ); }
};
} // namespace std
