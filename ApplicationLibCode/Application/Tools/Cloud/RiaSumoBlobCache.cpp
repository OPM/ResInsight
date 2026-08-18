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

#include "RiaSumoBlobCache.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSumoBlobCache::RiaSumoBlobCache( size_t limitBytes )
    : m_limitBytes( limitBytes )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaSumoBlobCache::contains( const QString& key ) const
{
    return m_entries.contains( key );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QByteArray RiaSumoBlobCache::lookup( const QString& key )
{
    auto it = m_entries.find( key );
    if ( it == m_entries.end() ) return {};

    m_order.splice( m_order.begin(), m_order, it->second.orderIterator );

    return it->second.contents;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoBlobCache::insert( const QString& key, const QByteArray& contents )
{
    if ( contents.isEmpty() ) return;

    const size_t contentsSize = static_cast<size_t>( contents.size() );

    // A blob larger than the whole cache would evict everything else to make room for itself. Leave it
    // uncached instead, so the smaller blobs already present stay available.
    if ( contentsSize > m_limitBytes ) return;

    // Re-inserting an existing key would leak its order list entry, so drop the previous version first.
    if ( auto it = m_entries.find( key ); it != m_entries.end() )
    {
        m_sizeBytes -= static_cast<size_t>( it->second.contents.size() );
        m_order.erase( it->second.orderIterator );
        m_entries.erase( it );
    }

    m_order.push_front( key );
    m_entries[key] = { contents, m_order.begin() };
    m_sizeBytes += contentsSize;

    while ( m_sizeBytes > m_limitBytes && !m_order.empty() )
    {
        const QString& oldestKey = m_order.back();

        if ( auto it = m_entries.find( oldestKey ); it != m_entries.end() )
        {
            m_sizeBytes -= static_cast<size_t>( it->second.contents.size() );
            m_entries.erase( it );
        }

        m_order.pop_back();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSumoBlobCache::clear()
{
    m_entries.clear();
    m_order.clear();
    m_sizeBytes = 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RiaSumoBlobCache::sizeBytes() const
{
    return m_sizeBytes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RiaSumoBlobCache::entryCount() const
{
    return m_entries.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RiaSumoBlobCache::limitBytes() const
{
    return m_limitBytes;
}
