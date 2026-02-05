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
{
    // Pre-allocate empty string at index 0
    m_strings.push_back( "" );
    m_stringToIndex[""] = 0;
    m_emptyIndex        = 0;
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

    IndexType newIndex = static_cast<IndexType>( m_strings.size() );
    m_strings.push_back( str );
    m_stringToIndex[m_strings.back()] = newIndex;
    return newIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::string& RiaStringPool::getString( IndexType index ) const
{
    std::shared_lock<std::shared_mutex> lock( m_mutex );

    if ( index >= m_strings.size() )
    {
        throw std::out_of_range( "String pool index out of range" );
    }
    return m_strings[index];
}
