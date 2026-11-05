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

#include <algorithm>
#include <atomic>
#include <cstring>
#include <limits>
#include <memory>
#include <numeric>
#include <utility>

namespace
{
//--------------------------------------------------------------------------------------------------
/// Bit pattern of a coordinate, with -0.0 normalized to 0.0
//--------------------------------------------------------------------------------------------------
uint64_t coordinateBits( double value )
{
    value += 0.0;
    uint64_t bits;
    std::memcpy( &bits, &value, sizeof( bits ) );
    return bits;
}

//--------------------------------------------------------------------------------------------------
/// Nodes are equal if their coordinates have equal bit patterns. This matches operator== except for NaN, so that
/// NaN nodes with the same bits are merged instead of each probing past all the earlier ones.
//--------------------------------------------------------------------------------------------------
bool isSameNode( const cvf::Vec3d& a, const cvf::Vec3d& b )
{
    return coordinateBits( a.x() ) == coordinateBits( b.x() ) && coordinateBits( a.y() ) == coordinateBits( b.y() ) &&
           coordinateBits( a.z() ) == coordinateBits( b.z() );
}

//--------------------------------------------------------------------------------------------------
/// Hash of the normalized bit pattern of a node, consistent with isSameNode().
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
        // splitmix64 finalizer
        uint64_t z = h ^ coordinateBits( value );
        z          = ( z ^ ( z >> 30 ) ) * 0xbf58476d1ce4e5b9ULL;
        z          = ( z ^ ( z >> 27 ) ) * 0x94d049bb133111ebULL;
        return z ^ ( z >> 31 );
    };

    return mix( mix( mix( 0x9e3779b97f4a7c15ULL, p.x() ), p.y() ), p.z() );
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// Nodes with exactly equal coordinates (see isSameNode()) are merged. The first occurrence of a coordinate is kept, and the
/// relative order of the kept nodes is preserved, so identical input gives an identical node table.
///
/// Returns the mapping from old to new node index. The mapping is empty, and the nodes are left as they are,
/// if no nodes were removed or there are 2^31 nodes or more.
//--------------------------------------------------------------------------------------------------
std::vector<uint32_t> RigGridNodeDeduplication::removeDuplicateNodes( std::vector<cvf::Vec3d>& nodes )
{
    const size_t oldNodeCount = nodes.size();

    // 32-bit indices keep the temporary memory low, and the top bit marks duplicates in oldToNew
    using IndexType                  = uint32_t;
    constexpr IndexType duplicateBit = IndexType( 1 ) << 31;
    constexpr IndexType emptySlot    = std::numeric_limits<IndexType>::max();
    if ( oldNodeCount == 0 || oldNodeCount >= duplicateBit ) return {};

    // Open addressing table of node indices, with a load factor of at most 2/3
    size_t capacity = 1;
    while ( capacity < oldNodeCount + oldNodeCount / 2 )
    {
        capacity *= 2;
    }
    const size_t mask = capacity - 1;

    // Work is split in fixed size blocks, so the result does not depend on the number of threads
    constexpr size_t blockSize  = 16384;
    auto             blockCount = []( size_t count ) { return static_cast<long>( ( count + blockSize - 1 ) / blockSize ); };
    auto             blockRange = [&]( long block, size_t count )
    { return std::pair{ block * blockSize, std::min( ( block + 1 ) * blockSize, count ) }; };

    const long nodeBlockCount  = blockCount( oldNodeCount );
    const long tableBlockCount = blockCount( capacity );

    std::unique_ptr<IndexType[]> table( new IndexType[capacity] );
#pragma omp parallel for
    for ( long block = 0; block < tableBlockCount; block++ )
    {
        const auto [begin, end] = blockRange( block, capacity );
        std::fill( table.get() + begin, table.get() + end, emptySlot );
    }

    // Pass 1: Insert all nodes concurrently. A filled slot keeps its coordinate, and ends up holding the lowest
    // index of the nodes with that coordinate. oldToNew temporarily holds the slot of each node.
    std::vector<IndexType> oldToNew( oldNodeCount );
#pragma omp parallel for
    for ( long block = 0; block < nodeBlockCount; block++ )
    {
        const auto [begin, end] = blockRange( block, oldNodeCount );
        for ( size_t nodeIndex = begin; nodeIndex < end; nodeIndex++ )
        {
            const cvf::Vec3d& node  = nodes[nodeIndex];
            const auto        index = static_cast<IndexType>( nodeIndex );

            size_t slot = nodeHash( node ) & mask;
            while ( true )
            {
                std::atomic_ref<IndexType> entry( table[slot] );

                IndexType current = entry.load( std::memory_order_relaxed );
                if ( current == emptySlot && entry.compare_exchange_strong( current, index, std::memory_order_relaxed ) ) break;

                if ( isSameNode( nodes[current], node ) )
                {
                    while ( index < current && !entry.compare_exchange_weak( current, index, std::memory_order_relaxed ) )
                    {
                    }
                    break;
                }

                slot = ( slot + 1 ) & mask;
            }

            oldToNew[nodeIndex] = static_cast<IndexType>( slot );
        }
    }

    // Pass 2: Replace the slot with the first node of the same coordinate, marking duplicates, and count the
    // unique nodes of each block
    std::vector<size_t> blockOffsets( nodeBlockCount + 1, 0 );
#pragma omp parallel for
    for ( long block = 0; block < nodeBlockCount; block++ )
    {
        const auto [begin, end] = blockRange( block, oldNodeCount );
        for ( size_t nodeIndex = begin; nodeIndex < end; nodeIndex++ )
        {
            const IndexType firstIndex = table[oldToNew[nodeIndex]];
            if ( firstIndex == nodeIndex )
            {
                blockOffsets[block + 1]++;
            }
            else
            {
                oldToNew[nodeIndex] = firstIndex | duplicateBit;
            }
        }
    }
    table.reset();

    std::partial_sum( blockOffsets.begin(), blockOffsets.end(), blockOffsets.begin() );
    const size_t newNodeCount = blockOffsets.back();
    if ( newNodeCount == oldNodeCount ) return {};

    // Pass 3: Number and copy the unique nodes in their original order
    std::vector<cvf::Vec3d> newNodes( newNodeCount );
#pragma omp parallel for
    for ( long block = 0; block < nodeBlockCount; block++ )
    {
        const auto [begin, end] = blockRange( block, oldNodeCount );
        auto newIndex           = static_cast<IndexType>( blockOffsets[block] );
        for ( size_t nodeIndex = begin; nodeIndex < end; nodeIndex++ )
        {
            if ( ( oldToNew[nodeIndex] & duplicateBit ) == 0 )
            {
                newNodes[newIndex]  = nodes[nodeIndex];
                oldToNew[nodeIndex] = newIndex++;
            }
        }
    }

    // Pass 4: Duplicates get the new index of the first node, which was resolved in pass 3
#pragma omp parallel for
    for ( long block = 0; block < nodeBlockCount; block++ )
    {
        const auto [begin, end] = blockRange( block, oldNodeCount );
        for ( size_t nodeIndex = begin; nodeIndex < end; nodeIndex++ )
        {
            const IndexType value = oldToNew[nodeIndex];
            if ( value & duplicateBit ) oldToNew[nodeIndex] = oldToNew[value & ~duplicateBit];
        }
    }

    nodes = std::move( newNodes );

    return oldToNew;
}
