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

#include "RiaStringPool.h"

#include <mutex>
#include <stdexcept>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaStringPool& RiaStringPool::instance()
{
    static RiaStringPool pool;
    return pool;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaStringPool::RiaStringPool()
    : m_chunks( new std::atomic<std::string*>[MAX_CHUNKS] )
    , m_size( 0 )
    , m_emptyIndex( 0 )
{
    for ( size_t i = 0; i < MAX_CHUNKS; i++ )
    {
        m_chunks[i].store( nullptr, std::memory_order_relaxed );
    }

    // Pre-allocate empty string at index 0. Insert in the map first so a failure cannot leak a chunk.
    m_stringToIndex[""] = 0;
    addString( "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaStringPool::~RiaStringPool()
{
    for ( size_t i = 0; i < MAX_CHUNKS; i++ )
    {
        delete[] m_chunks[i].load( std::memory_order_relaxed );
    }
}

//--------------------------------------------------------------------------------------------------
/// Must be called with the exclusive lock held (or during construction)
//--------------------------------------------------------------------------------------------------
RiaStringPool::IndexType RiaStringPool::addString( const std::string& str )
{
    const IndexType newIndex = m_size.load( std::memory_order_relaxed );
    const size_t    chunkIdx = newIndex >> CHUNK_BITS;
    if ( chunkIdx >= MAX_CHUNKS ) throw std::length_error( "String pool is full" );

    std::string* chunk = m_chunks[chunkIdx].load( std::memory_order_relaxed );
    if ( !chunk )
    {
        chunk = new std::string[CHUNK_SIZE];
        m_chunks[chunkIdx].store( chunk, std::memory_order_release );
    }
    chunk[newIndex & CHUNK_MASK] = str;

    // Publish the string. Readers only access indices below the size.
    m_size.store( newIndex + 1, std::memory_order_release );
    return newIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaStringPool::IndexType RiaStringPool::getIndex( const std::string& str )
{
    // First try with shared lock for reading
    {
        std::shared_lock<std::shared_mutex> lock( m_mutex );
        auto                                it = m_stringToIndex.find( str );
        if ( it != m_stringToIndex.end() )
        {
            return it->second;
        }
    }

    // Need to insert, acquire exclusive lock
    std::unique_lock<std::shared_mutex> lock( m_mutex );

    // Double-check after acquiring exclusive lock (another thread might have inserted it)
    auto it = m_stringToIndex.find( str );
    if ( it != m_stringToIndex.end() )
    {
        return it->second;
    }

    IndexType newIndex   = addString( str );
    m_stringToIndex[str] = newIndex;
    return newIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::string& RiaStringPool::getString( IndexType index ) const
{
    if ( index >= m_size.load( std::memory_order_acquire ) )
    {
        throw std::out_of_range( "String pool index out of range" );
    }

    const std::string* chunk = m_chunks[index >> CHUNK_BITS].load( std::memory_order_relaxed );
    return chunk[index & CHUNK_MASK];
}
