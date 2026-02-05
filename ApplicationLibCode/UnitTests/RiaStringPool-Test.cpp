#include "gtest/gtest.h"

#include "RiaStringPool.h"

#include <algorithm>
#include <set>
#include <thread>
#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaStringPoolTest, EmptyStringPreAllocated )
{
    auto& pool = RiaStringPool::instance();

    auto emptyIndex = pool.getIndex( "" );
    EXPECT_EQ( pool.getEmptyIndex(), emptyIndex );
    EXPECT_EQ( "", pool.getString( emptyIndex ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaStringPoolTest, BasicStringStorage )
{
    auto& pool = RiaStringPool::instance();

    std::string testStr = "FOPT";
    auto        index   = pool.getIndex( testStr );

    EXPECT_EQ( testStr, pool.getString( index ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaStringPoolTest, DuplicateStringsReturnSameIndex )
{
    auto& pool = RiaStringPool::instance();

    std::string testStr = "WOPT:WELL1";
    auto        index1  = pool.getIndex( testStr );
    auto        index2  = pool.getIndex( testStr );
    auto        index3  = pool.getIndex( testStr );

    EXPECT_EQ( index1, index2 );
    EXPECT_EQ( index2, index3 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaStringPoolTest, MultipleUniqueStrings )
{
    auto& pool = RiaStringPool::instance();

    std::vector<std::string> testStrings = { "FOPT", "WOPT:WELL1", "GOPT:GROUP1", "BPR:10,20,30", "ROPR" };

    std::vector<RiaStringPool::IndexType> indices;
    for ( const auto& str : testStrings )
    {
        indices.push_back( pool.getIndex( str ) );
    }

    // All indices should be unique
    std::set<RiaStringPool::IndexType> uniqueIndices( indices.begin(), indices.end() );
    EXPECT_EQ( testStrings.size(), uniqueIndices.size() );

    // Verify retrieval
    for ( size_t i = 0; i < testStrings.size(); ++i )
    {
        EXPECT_EQ( testStrings[i], pool.getString( indices[i] ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaStringPoolTest, OutOfRangeThrows )
{
    auto& pool = RiaStringPool::instance();

    // Attempt to access index far beyond current size
    EXPECT_THROW( pool.getString( 9999999 ), std::out_of_range );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaStringPoolTest, ThreadSafety )
{
    auto& pool = RiaStringPool::instance();

    const int numThreads       = 10;
    const int stringsPerThread = 100;

    std::vector<std::thread>                         threads;
    std::vector<std::vector<RiaStringPool::IndexType>> threadResults( numThreads );

    // Create threads that all add the same strings
    for ( int t = 0; t < numThreads; ++t )
    {
        threads.emplace_back(
            [&pool, &threadResults, t, stringsPerThread]()
            {
                for ( int i = 0; i < stringsPerThread; ++i )
                {
                    std::string str   = "STRING_" + std::to_string( i );
                    auto        index = pool.getIndex( str );
                    threadResults[t].push_back( index );
                }
            } );
    }

    // Wait for all threads to complete
    for ( auto& thread : threads )
    {
        thread.join();
    }

    // Verify all threads got the same indices for the same strings
    for ( int i = 1; i < numThreads; ++i )
    {
        EXPECT_EQ( threadResults[0].size(), threadResults[i].size() );
        for ( size_t j = 0; j < threadResults[0].size(); ++j )
        {
            EXPECT_EQ( threadResults[0][j], threadResults[i][j] );
        }
    }

    // Verify string retrieval
    for ( int i = 0; i < stringsPerThread; ++i )
    {
        std::string expectedStr = "STRING_" + std::to_string( i );
        auto        index       = threadResults[0][i];
        EXPECT_EQ( expectedStr, pool.getString( index ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaStringPoolTest, LargeNumberOfStrings )
{
    auto& pool = RiaStringPool::instance();

    const int                             numStrings = 10000;
    std::vector<RiaStringPool::IndexType> indices;

    // Add many unique strings
    for ( int i = 0; i < numStrings; ++i )
    {
        std::string str = "VECTOR_" + std::to_string( i ) + "_DATA";
        indices.push_back( pool.getIndex( str ) );
    }

    // Verify all can be retrieved
    for ( int i = 0; i < numStrings; ++i )
    {
        std::string expectedStr = "VECTOR_" + std::to_string( i ) + "_DATA";
        EXPECT_EQ( expectedStr, pool.getString( indices[i] ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaStringPoolTest, SpecialCharacters )
{
    auto& pool = RiaStringPool::instance();

    std::vector<std::string> specialStrings = { "String with spaces",
                                                "String:with:colons",
                                                "String/with/slashes",
                                                "String,with,commas",
                                                "String-with-dashes",
                                                "String_with_underscores",
                                                "String.with.dots",
                                                "String|with|pipes",
                                                "String@with@at",
                                                "String#with#hash" };

    for ( const auto& str : specialStrings )
    {
        auto index = pool.getIndex( str );
        EXPECT_EQ( str, pool.getString( index ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaStringPoolTest, VeryLongStrings )
{
    auto& pool = RiaStringPool::instance();

    // Create a very long string
    std::string longStr( 10000, 'A' );
    longStr += "_UNIQUE";

    auto index = pool.getIndex( longStr );
    EXPECT_EQ( longStr, pool.getString( index ) );

    // Verify same index on duplicate
    auto index2 = pool.getIndex( longStr );
    EXPECT_EQ( index, index2 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaStringPoolTest, ConcurrentInsertAndRead )
{
    auto& pool = RiaStringPool::instance();

    const int numWriteThreads = 5;
    const int numReadThreads  = 5;
    const int stringsPerWrite = 50;

    std::vector<std::thread> threads;

    // Create strings in multiple write threads
    for ( int t = 0; t < numWriteThreads; ++t )
    {
        threads.emplace_back(
            [&pool, t, stringsPerWrite]()
            {
                for ( int i = 0; i < stringsPerWrite; ++i )
                {
                    std::string str = "WRITE_" + std::to_string( t ) + "_" + std::to_string( i );
                    pool.getIndex( str );
                }
            } );
    }

    // Read strings concurrently
    for ( int t = 0; t < numReadThreads; ++t )
    {
        threads.emplace_back(
            [&pool, t, stringsPerWrite]()
            {
                for ( int i = 0; i < stringsPerWrite; ++i )
                {
                    // Try to read strings that might be being created
                    std::string str   = "WRITE_0_" + std::to_string( i );
                    auto        index = pool.getIndex( str );
                    // Should not crash even if racing with writes
                    EXPECT_EQ( str, pool.getString( index ) );
                }
            } );
    }

    for ( auto& thread : threads )
    {
        thread.join();
    }

    // Verify all written strings are accessible
    for ( int t = 0; t < numWriteThreads; ++t )
    {
        for ( int i = 0; i < stringsPerWrite; ++i )
        {
            std::string str   = "WRITE_" + std::to_string( t ) + "_" + std::to_string( i );
            auto        index = pool.getIndex( str );
            EXPECT_EQ( str, pool.getString( index ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaStringPoolTest, InvalidIndex )
{
    auto& pool = RiaStringPool::instance();

    // Test with INVALID_INDEX constant
    EXPECT_THROW( pool.getString( RiaStringPool::INVALID_INDEX ), std::out_of_range );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaStringPoolTest, SimilarStrings )
{
    auto& pool = RiaStringPool::instance();

    // Test strings that are similar but not identical
    std::vector<std::string> similarStrings = { "WOPT:WELL-1",
                                                "WOPT:WELL-2",
                                                "WOPT:WELL-10",
                                                "WOPT:WELL-1A",
                                                "WOPT:WELL_1",
                                                "WOPT: WELL-1" };

    std::vector<RiaStringPool::IndexType> indices;
    for ( const auto& str : similarStrings )
    {
        indices.push_back( pool.getIndex( str ) );
    }

    // All should have different indices
    std::set<RiaStringPool::IndexType> uniqueIndices( indices.begin(), indices.end() );
    EXPECT_EQ( similarStrings.size(), uniqueIndices.size() );

    // Verify each string retrieval
    for ( size_t i = 0; i < similarStrings.size(); ++i )
    {
        EXPECT_EQ( similarStrings[i], pool.getString( indices[i] ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaStringPoolTest, CommonSummaryVectorNames )
{
    auto& pool = RiaStringPool::instance();

    // Test with realistic summary vector names
    std::vector<std::string> vectorNames = { "FOPT",
                                             "FWPT",
                                             "FGPT",
                                             "FOPR",
                                             "FWPR",
                                             "FGPR",
                                             "WOPT:PROD-1",
                                             "WWPT:PROD-1",
                                             "WBHP:PROD-1",
                                             "WOPT:INJ-1",
                                             "GOPT:FIELD",
                                             "BPR:10,15,20" };

    // Store indices
    std::vector<RiaStringPool::IndexType> indices;
    for ( const auto& name : vectorNames )
    {
        indices.push_back( pool.getIndex( name ) );
    }

    // Request same strings again - should get same indices
    for ( size_t i = 0; i < vectorNames.size(); ++i )
    {
        auto index = pool.getIndex( vectorNames[i] );
        EXPECT_EQ( indices[i], index );
        EXPECT_EQ( vectorNames[i], pool.getString( index ) );
    }
}
