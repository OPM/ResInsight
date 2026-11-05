/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RigGridNodeDeduplication.h"

#include <cstring>
#include <limits>

namespace
{
//--------------------------------------------------------------------------------------------------
/// Hash of the exact bit pattern of a coordinate. -0.0 is normalized to 0.0 so it hashes like 0.0,
/// matching operator==.
///
/// Performance: this is on the hot path of removeDuplicateNodes(). Each coordinate is mixed with the
/// branch-free splitmix64 finalizer, giving well distributed low bits for the power-of-two open addressing
/// table. std::hash<double> was measured to be about 2.4x slower to compute and gave about 80% slower
/// deduplication of 78.4M grid nodes (MSVC, /O2), as MSVC hashes the bytes of each double with FNV-1a and
/// three hashes must be combined. Do not replace without benchmarking.
///
/// The finalizer is taken from splitmix64 by Sebastiano Vigna (public domain),
/// https://prng.di.unimi.it/splitmix64.c, which uses the mixing constants of David Stafford's Mix13,
/// http://zimbry.blogspot.com/2011/09/better-bit-mixing-improving-on.html
/// The seed 0x9e3779b97f4a7c15 is the 64-bit golden ratio increment used by splitmix64.
//--------------------------------------------------------------------------------------------------
uint64_t nodeHash( const cvf::Vec3d& p )
{
    auto mix = []( uint64_t h, double value )
    {
        value += 0.0;
        uint64_t bits;
        std::memcpy( &bits, &value, sizeof( bits ) );

        // splitmix64 finalizer
        uint64_t z = h ^ bits;
        z          = ( z ^ ( z >> 30 ) ) * 0xbf58476d1ce4e5b9ULL;
        z          = ( z ^ ( z >> 27 ) ) * 0x94d049bb133111ebULL;
        return z ^ ( z >> 31 );
    };

    return mix( mix( mix( 0x9e3779b97f4a7c15ULL, p.x() ), p.y() ), p.z() );
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// Nodes with exactly equal coordinates are merged. The first occurrence of a coordinate is kept, and the
/// relative order of the kept nodes is preserved, so identical input gives an identical node table.
///
/// Returns the mapping from old to new node index. The mapping is empty, and the nodes are left as they are,
/// if no nodes were removed or there are 2^32 - 1 nodes or more.
//--------------------------------------------------------------------------------------------------
std::vector<uint32_t> RigGridNodeDeduplication::removeDuplicateNodes( std::vector<cvf::Vec3d>& nodes )
{
    const size_t oldNodeCount = nodes.size();

    // 32-bit indices keep the temporary memory low
    using IndexType               = uint32_t;
    constexpr IndexType emptySlot = std::numeric_limits<IndexType>::max();
    if ( oldNodeCount == 0 || oldNodeCount >= emptySlot ) return {};

    // Open addressing table of new node indices, with a load factor of at most 2/3
    size_t capacity = 1;
    while ( capacity < oldNodeCount + oldNodeCount / 2 )
    {
        capacity *= 2;
    }
    const size_t mask = capacity - 1;

    std::vector<IndexType> table( capacity, emptySlot );
    std::vector<IndexType> oldToNew( oldNodeCount );

    // Compact in place: newIndex <= oldIndex, so a node is read before its position can be overwritten
    IndexType newNodeCount = 0;
    for ( size_t oldIndex = 0; oldIndex < oldNodeCount; oldIndex++ )
    {
        const cvf::Vec3d node = nodes[oldIndex];

        size_t slot = nodeHash( node ) & mask;
        while ( table[slot] != emptySlot && !( nodes[table[slot]] == node ) )
        {
            slot = ( slot + 1 ) & mask;
        }

        if ( table[slot] == emptySlot )
        {
            table[slot]           = newNodeCount;
            nodes[newNodeCount++] = node;
        }

        oldToNew[oldIndex] = table[slot];
    }

    if ( newNodeCount == oldNodeCount ) return {};

    table.clear();
    table.shrink_to_fit();

    nodes.resize( newNodeCount );
    nodes.shrink_to_fit();

    return oldToNew;
}
