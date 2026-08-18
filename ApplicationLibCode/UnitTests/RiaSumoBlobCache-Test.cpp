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

#include "gtest/gtest.h"

#include "Cloud/RiaSumoBlobCache.h"

namespace
{
QByteArray blobOfSize( int size, char fill = 'x' )
{
    return QByteArray( size, fill );
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaSumoBlobCacheTest, LookupOfMissingKeyReturnsEmpty )
{
    RiaSumoBlobCache cache( 100 );

    EXPECT_FALSE( cache.contains( "missing" ) );
    EXPECT_TRUE( cache.lookup( "missing" ).isEmpty() );
    EXPECT_EQ( size_t( 0 ), cache.entryCount() );
    EXPECT_EQ( size_t( 0 ), cache.sizeBytes() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaSumoBlobCacheTest, InsertedBlobIsReturnedUnchanged )
{
    RiaSumoBlobCache cache( 100 );

    const QByteArray contents = blobOfSize( 10, 'a' );
    cache.insert( "key", contents );

    EXPECT_TRUE( cache.contains( "key" ) );
    EXPECT_EQ( contents, cache.lookup( "key" ) );
    EXPECT_EQ( size_t( 1 ), cache.entryCount() );
    EXPECT_EQ( size_t( 10 ), cache.sizeBytes() );
}

//--------------------------------------------------------------------------------------------------
/// An empty array is how a cache miss is reported, so it must never be stored as a value.
//--------------------------------------------------------------------------------------------------
TEST( RiaSumoBlobCacheTest, EmptyContentsAreNotCached )
{
    RiaSumoBlobCache cache( 100 );

    cache.insert( "key", QByteArray() );

    EXPECT_FALSE( cache.contains( "key" ) );
    EXPECT_EQ( size_t( 0 ), cache.entryCount() );
}

//--------------------------------------------------------------------------------------------------
/// Caching a blob bigger than the whole limit would evict everything else to make room for it.
//--------------------------------------------------------------------------------------------------
TEST( RiaSumoBlobCacheTest, BlobLargerThanLimitIsNotCachedAndKeepsExistingEntries )
{
    RiaSumoBlobCache cache( 100 );

    cache.insert( "small", blobOfSize( 10 ) );
    cache.insert( "huge", blobOfSize( 101 ) );

    EXPECT_FALSE( cache.contains( "huge" ) );
    EXPECT_TRUE( cache.contains( "small" ) );
    EXPECT_EQ( size_t( 1 ), cache.entryCount() );
    EXPECT_EQ( size_t( 10 ), cache.sizeBytes() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaSumoBlobCacheTest, BlobExactlyAtLimitIsCached )
{
    RiaSumoBlobCache cache( 100 );

    cache.insert( "exact", blobOfSize( 100 ) );

    EXPECT_TRUE( cache.contains( "exact" ) );
    EXPECT_EQ( size_t( 100 ), cache.sizeBytes() );
}

//--------------------------------------------------------------------------------------------------
/// Exceeding the limit evicts from the back of the recency order, oldest first.
//--------------------------------------------------------------------------------------------------
TEST( RiaSumoBlobCacheTest, LeastRecentlyUsedIsEvictedFirst )
{
    RiaSumoBlobCache cache( 100 );

    cache.insert( "a", blobOfSize( 40 ) );
    cache.insert( "b", blobOfSize( 40 ) );
    cache.insert( "c", blobOfSize( 40 ) ); // pushes the total to 120, so "a" has to go

    EXPECT_FALSE( cache.contains( "a" ) );
    EXPECT_TRUE( cache.contains( "b" ) );
    EXPECT_TRUE( cache.contains( "c" ) );
    EXPECT_EQ( size_t( 80 ), cache.sizeBytes() );
}

//--------------------------------------------------------------------------------------------------
/// A lookup counts as use, so the blob it returns must survive the next eviction.
//--------------------------------------------------------------------------------------------------
TEST( RiaSumoBlobCacheTest, LookupRefreshesRecency )
{
    RiaSumoBlobCache cache( 100 );

    cache.insert( "a", blobOfSize( 40 ) );
    cache.insert( "b", blobOfSize( 40 ) );

    cache.lookup( "a" ); // "b" is now the least recently used

    cache.insert( "c", blobOfSize( 40 ) );

    EXPECT_TRUE( cache.contains( "a" ) );
    EXPECT_FALSE( cache.contains( "b" ) );
    EXPECT_TRUE( cache.contains( "c" ) );
}

//--------------------------------------------------------------------------------------------------
/// Re-inserting a key must replace the entry rather than leave a stale recency entry behind, and the
/// accounted size must follow the new contents.
//--------------------------------------------------------------------------------------------------
TEST( RiaSumoBlobCacheTest, ReinsertingKeyReplacesEntryAndSize )
{
    RiaSumoBlobCache cache( 100 );

    cache.insert( "key", blobOfSize( 10, 'a' ) );
    cache.insert( "key", blobOfSize( 30, 'b' ) );

    EXPECT_EQ( size_t( 1 ), cache.entryCount() );
    EXPECT_EQ( size_t( 30 ), cache.sizeBytes() );
    EXPECT_EQ( blobOfSize( 30, 'b' ), cache.lookup( "key" ) );

    // A stale recency entry would make this eviction drop the wrong key, or drop nothing at all.
    cache.insert( "other", blobOfSize( 80 ) );

    EXPECT_FALSE( cache.contains( "key" ) );
    EXPECT_TRUE( cache.contains( "other" ) );
    EXPECT_EQ( size_t( 1 ), cache.entryCount() );
    EXPECT_EQ( size_t( 80 ), cache.sizeBytes() );
}

//--------------------------------------------------------------------------------------------------
/// One insert may have to evict several entries to get back within the limit.
//--------------------------------------------------------------------------------------------------
TEST( RiaSumoBlobCacheTest, InsertEvictsAsManyEntriesAsNeeded )
{
    RiaSumoBlobCache cache( 100 );

    cache.insert( "a", blobOfSize( 20 ) );
    cache.insert( "b", blobOfSize( 20 ) );
    cache.insert( "c", blobOfSize( 20 ) );
    cache.insert( "big", blobOfSize( 90 ) );

    EXPECT_TRUE( cache.contains( "big" ) );
    EXPECT_FALSE( cache.contains( "a" ) );
    EXPECT_FALSE( cache.contains( "b" ) );
    EXPECT_EQ( size_t( 1 ), cache.entryCount() );
    EXPECT_EQ( size_t( 90 ), cache.sizeBytes() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaSumoBlobCacheTest, ClearEmptiesTheCache )
{
    RiaSumoBlobCache cache( 100 );

    cache.insert( "a", blobOfSize( 20 ) );
    cache.insert( "b", blobOfSize( 20 ) );

    cache.clear();

    EXPECT_EQ( size_t( 0 ), cache.entryCount() );
    EXPECT_EQ( size_t( 0 ), cache.sizeBytes() );
    EXPECT_FALSE( cache.contains( "a" ) );

    // The recency order must be cleared too, otherwise a later insert evicts against stale keys.
    cache.insert( "c", blobOfSize( 20 ) );
    EXPECT_TRUE( cache.contains( "c" ) );
    EXPECT_EQ( size_t( 20 ), cache.sizeBytes() );
}
