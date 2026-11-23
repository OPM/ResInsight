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

#include <atomic>
#include <cstdint>
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>

//==================================================================================================
/// String pool for memory-efficient storage of shared strings
/// Stores strings once and returns indices for lookups
///
/// Strings are stored in fixed-size chunks that are never moved or modified after they are
/// published. This makes getString() lock-free, and the returned references stay valid for the
/// lifetime of the pool, also when other threads add strings.
//==================================================================================================
class RiaStringPool
{
public:
    using IndexType                          = uint32_t;
    static constexpr IndexType INVALID_INDEX = static_cast<IndexType>( -1 );

    static RiaStringPool& instance();

    // Get or create an index for a string
    IndexType getIndex( const std::string& str );

    // Get string by index. Lock-free, and the reference is valid for the lifetime of the pool.
    const std::string& getString( IndexType index ) const;

    // Get empty string index
    IndexType getEmptyIndex() const { return m_emptyIndex; }

private:
    RiaStringPool();
    ~RiaStringPool();
    IndexType addString( const std::string& str );
    RiaStringPool( const RiaStringPool& )            = delete;
    RiaStringPool& operator=( const RiaStringPool& ) = delete;

    static constexpr size_t CHUNK_BITS = 12;
    static constexpr size_t CHUNK_SIZE = size_t( 1 ) << CHUNK_BITS;
    static constexpr size_t CHUNK_MASK = CHUNK_SIZE - 1;
    static constexpr size_t MAX_CHUNKS = size_t( 1 ) << 16;

    mutable std::shared_mutex                    m_mutex; // protects m_stringToIndex and string insertion
    std::unique_ptr<std::atomic<std::string*>[]> m_chunks;
    std::atomic<IndexType>                       m_size;
    std::unordered_map<std::string, IndexType>   m_stringToIndex;
    IndexType                                    m_emptyIndex;
};
