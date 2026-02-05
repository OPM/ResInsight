/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include <cstdint>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

//==================================================================================================
/// String pool for memory-efficient storage of shared strings
/// Stores strings once and returns indices for lookups
//==================================================================================================
class RiaStringPool
{
public:
    using IndexType                          = uint32_t;
    static constexpr IndexType INVALID_INDEX = static_cast<IndexType>( -1 );

    static RiaStringPool& instance();

    // Get or create an index for a string
    IndexType getIndex( const std::string& str );

    // Get string by index
    const std::string& getString( IndexType index ) const;

    // Get empty string index
    IndexType getEmptyIndex() const { return m_emptyIndex; }

private:
    RiaStringPool();
    RiaStringPool( const RiaStringPool& )            = delete;
    RiaStringPool& operator=( const RiaStringPool& ) = delete;

    mutable std::shared_mutex                  m_mutex;
    std::vector<std::string>                   m_strings;
    std::unordered_map<std::string, IndexType> m_stringToIndex;
    IndexType                                  m_emptyIndex;
};
