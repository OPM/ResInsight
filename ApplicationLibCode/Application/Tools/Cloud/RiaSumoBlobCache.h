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

#include <QByteArray>
#include <QString>

#include <cstddef>
#include <list>
#include <map>

//==================================================================================================
/// Blobs downloaded from Sumo, kept in memory so a repeated request is answered without going to the
/// server. Displaying a grid property computes its legend range across all time steps and then reads the
/// values again, so the same blob is asked for several times.
///
/// Bounded by total byte size rather than entry count, as blob sizes follow the grid size and vary by
/// orders of magnitude. The least recently used entries are evicted when the limit is exceeded.
//==================================================================================================
class RiaSumoBlobCache
{
public:
    explicit RiaSumoBlobCache( size_t limitBytes );

    bool contains( const QString& key ) const;

    // The cached contents, or an empty array when the key is not cached. A hit is moved to the front of
    // the recency order, so it is evicted last.
    QByteArray lookup( const QString& key );

    // Empty contents are not cached, as an empty array is how a miss is reported. Contents larger than the
    // whole limit are not cached either, as making room for them would evict everything else.
    void insert( const QString& key, const QByteArray& contents );

    void clear();

    size_t sizeBytes() const;
    size_t entryCount() const;
    size_t limitBytes() const;

private:
    struct Entry
    {
        QByteArray                   contents;
        std::list<QString>::iterator orderIterator;
    };

    std::map<QString, Entry> m_entries;
    std::list<QString>       m_order; // most recently used at front
    size_t                   m_sizeBytes = 0;
    const size_t             m_limitBytes;
};
