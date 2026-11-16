#include "gtest/gtest.h"
#include <cstdint>
#include <future>
#include <limits>

#include "KeyValueStore/RiaKeyValueStore.h"
#include "KeyValueStore/RiaKeyValueStoreUtil.h"

// Test set and get operations
TEST( RiaKeyValueStoreTest, SetAndGet )
{
    // Test get existing keys
    RiaKeyValueStore<int> intStore;
    intStore.set( "numbers", { 1, 2, 3, 4, 5 } );
    intStore.set( "even", { 2, 4, 6, 8 } );

    auto numbers = intStore.get( "numbers" );

    ASSERT_TRUE( numbers.has_value() );
    EXPECT_EQ( numbers->size(), 5 );
    EXPECT_EQ( ( *numbers )[0], 1 );
    EXPECT_EQ( ( *numbers )[4], 5 );

    RiaKeyValueStore<std::string> stringStore;
    stringStore.set( "fruits", { "apple", "banana", "cherry" } );
    stringStore.set( "colors", { "red", "green", "blue" } );

    auto fruits = stringStore.get( "fruits" );
    ASSERT_TRUE( fruits.has_value() );
    EXPECT_EQ( fruits->size(), 3 );
    EXPECT_EQ( ( *fruits )[0], "apple" );
    EXPECT_EQ( ( *fruits )[2], "cherry" );

    // Test get non-existing key
    EXPECT_FALSE( intStore.get( "prime" ).has_value() );
    EXPECT_FALSE( stringStore.get( "vegetables" ).has_value() );

    // Test set operation (overwrite)
    intStore.set( "numbers", { 10, 20, 30 } );
    auto updatedNumbers = intStore.get( "numbers" );
    ASSERT_TRUE( updatedNumbers.has_value() );
    EXPECT_EQ( updatedNumbers->size(), 3 );
    EXPECT_EQ( ( *updatedNumbers )[0], 10 );
    EXPECT_EQ( ( *updatedNumbers )[2], 30 );

    // Test set new key
    intStore.set( "prime", { 2, 3, 5, 7, 11 } );
    auto primes = intStore.get( "prime" );
    ASSERT_TRUE( primes.has_value() );
    EXPECT_EQ( primes->size(), 5 );
    EXPECT_EQ( ( *primes )[0], 2 );
    EXPECT_EQ( ( *primes )[4], 11 );
}

// Test remove operation
TEST( RiaKeyValueStoreTest, Remove )
{
    // Test remove existing key
    RiaKeyValueStore<int> intStore;
    auto                  numbers = intStore.get( "numbers" );
    intStore.set( "numbers", { 1, 2, 3, 4, 5 } );

    EXPECT_TRUE( intStore.exists( "numbers" ) );
    EXPECT_TRUE( intStore.remove( "numbers" ) );
    EXPECT_FALSE( intStore.exists( "numbers" ) );
    EXPECT_FALSE( intStore.get( "numbers" ).has_value() );

    // Test remove non-existing key
    EXPECT_FALSE( intStore.remove( "prime" ) );

    // Test remove already removed key
    EXPECT_FALSE( intStore.remove( "numbers" ) );
}

// Test exists operation
TEST( RiaKeyValueStoreTest, Exists )
{
    RiaKeyValueStore<int> intStore;
    auto                  numbers = intStore.get( "numbers" );
    intStore.set( "numbers", { 1, 2, 3, 4, 5 } );
    intStore.set( "even", { 2, 4, 6, 8 } );

    EXPECT_TRUE( intStore.exists( "numbers" ) );
    EXPECT_TRUE( intStore.exists( "even" ) );
    EXPECT_FALSE( intStore.exists( "prime" ) );
    EXPECT_FALSE( intStore.exists( "" ) ); // Empty key
}

// Test keys operation
TEST( RiaKeyValueStoreTest, Keys )
{
    RiaKeyValueStore<int> intStore;
    auto                  numbers = intStore.get( "numbers" );
    intStore.set( "numbers", { 1, 2, 3, 4, 5 } );
    intStore.set( "even", { 2, 4, 6, 8 } );

    auto intKeys = intStore.keys();
    EXPECT_EQ( intKeys.size(), 2 );
    // Since unordered_map doesn't guarantee order, we check if keys exist
    EXPECT_NE( std::find( intKeys.begin(), intKeys.end(), "numbers" ), intKeys.end() );
    EXPECT_NE( std::find( intKeys.begin(), intKeys.end(), "even" ), intKeys.end() );
    EXPECT_EQ( std::find( intKeys.begin(), intKeys.end(), "prime" ), intKeys.end() );
}

// Test clear operation
TEST( RiaKeyValueStoreTest, Clear )
{
    RiaKeyValueStore<int> intStore;
    auto                  numbers = intStore.get( "numbers" );
    intStore.set( "numbers", { 1, 2, 3, 4, 5 } );
    intStore.set( "even", { 2, 4, 6, 8 } );

    EXPECT_FALSE( intStore.keys().empty() );
    intStore.clear();
    EXPECT_TRUE( intStore.keys().empty() );
    EXPECT_FALSE( intStore.exists( "numbers" ) );
    EXPECT_FALSE( intStore.exists( "even" ) );
}

// Test edge cases
TEST( RiaKeyValueStoreTest, EdgeCases )
{
    RiaKeyValueStore<int> intStore;
    intStore.set( "empty", {} );
    auto emptyArray = intStore.get( "empty" );
    ASSERT_TRUE( emptyArray.has_value() );
    EXPECT_TRUE( emptyArray->empty() );
}

TEST( RiaKeyValueStoreTest, LargeArray )
{
    RiaKeyValueStore<int> intStore;
    std::vector<int>      largeArray( 10000, 42 );
    intStore.set( "large", largeArray );
    auto retrievedLarge = intStore.get( "large" );
    ASSERT_TRUE( retrievedLarge.has_value() );
    EXPECT_EQ( retrievedLarge->size(), 10000 );
    EXPECT_EQ( ( *retrievedLarge )[9999], 42 );
}

TEST( RiaKeyValueStoreTest, EmptyKey )
{
    RiaKeyValueStore<std::string> stringStore;
    stringStore.set( "", { "empty_key" } );
    auto emptyKey = stringStore.get( "" );
    ASSERT_TRUE( emptyKey.has_value() );
    EXPECT_EQ( ( *emptyKey )[0], "empty_key" );
}

// Concurrency test
TEST( RiaKeyValueStoreTest, BasicConcurrency )
{
    RiaKeyValueStore<int> store;
    constexpr int         numThreads          = 10;
    constexpr int         operationsPerThread = 100;

    // Function to run in each thread
    auto threadFunc = [&store]( int threadId )
    {
        for ( int i = 0; i < operationsPerThread; i++ )
        {
            std::string      key    = "thread_" + std::to_string( threadId ) + "_" + std::to_string( i );
            std::vector<int> values = { threadId, i, threadId * i };

            // Set values
            EXPECT_TRUE( store.set( key, values ) );

            // Get values
            auto result = store.get( key );
            EXPECT_TRUE( result.has_value() );
            if ( result.has_value() )
            {
                EXPECT_EQ( result->size(), 3 );
                EXPECT_EQ( ( *result )[0], threadId );
                EXPECT_EQ( ( *result )[1], i );
                EXPECT_EQ( ( *result )[2], threadId * i );
            }

            // Check existence
            EXPECT_TRUE( store.exists( key ) );

            // Remove some keys (every 3rd one)
            if ( i % 3 == 0 )
            {
                EXPECT_TRUE( store.remove( key ) );
                EXPECT_FALSE( store.exists( key ) );
            }
        }
    };

    // Start threads
    std::vector<std::future<void>> futures;
    for ( int i = 0; i < numThreads; i++ )
    {
        futures.push_back( std::async( std::launch::async, threadFunc, i ) );
    }

    // Wait for all threads to complete
    for ( auto& future : futures )
    {
        future.get();
    }

    // Validate final state
    int expectedRemainingKeys = numThreads * operationsPerThread * 2 / 3; // 2/3 of keys remain
    // Allow for some variation due to thread timing
    EXPECT_LE( store.keys().size(), expectedRemainingKeys );

    // Clear and check
    store.clear();
    EXPECT_TRUE( store.keys().empty() );
}

// Byte conversion for every element type the key-value store transports
template <typename T>
class RiaKeyValueStoreUtilTest : public testing::Test
{
};

using RiaKeyValueStoreElementTypes = testing::Types<float, double, int32_t, int64_t, uint32_t, uint64_t>;
TYPED_TEST_SUITE( RiaKeyValueStoreUtilTest, RiaKeyValueStoreElementTypes );

TYPED_TEST( RiaKeyValueStoreUtilTest, ConvertRoundTrip )
{
    using T = TypeParam;

    std::vector<T> values = { std::numeric_limits<T>::lowest(), T( 0 ), T( 1 ), T( 42 ), std::numeric_limits<T>::max() };

    std::vector<char> bytes = RiaKeyValueStoreUtil::convertToByteVector( values );
    EXPECT_EQ( bytes.size(), values.size() * sizeof( T ) );
    EXPECT_EQ( RiaKeyValueStoreUtil::convertToVector<T>( bytes ), values );

    RiaKeyValueStore<char> store;
    store.set( "values", bytes );
    EXPECT_EQ( RiaKeyValueStoreUtil::convertToVector<T>( store.get( "values" ) ), values );
}

TYPED_TEST( RiaKeyValueStoreUtilTest, ConvertInvalidInput )
{
    using T = TypeParam;

    EXPECT_TRUE( RiaKeyValueStoreUtil::convertToVector<T>( std::nullopt ).empty() );
    EXPECT_TRUE( RiaKeyValueStoreUtil::convertToVector<T>( std::vector<char>() ).empty() );
    EXPECT_TRUE( RiaKeyValueStoreUtil::convertToVector<T>( std::vector<char>( sizeof( T ) + 1 ) ).empty() );
    EXPECT_TRUE( RiaKeyValueStoreUtil::convertToByteVector( std::vector<T>() ).empty() );
}

TEST( RiaKeyValueStoreTest, ElementType )
{
    RiaKeyValueStore<char> store;

    // A coordinate that float32 cannot hold
    std::vector<double> coordinates = { 7319649.123 };
    store.set( "coordinates", RiaKeyValueStoreUtil::convertToByteVector( coordinates ), RiaKeyValueStoreElementType::FLOAT64 );
    EXPECT_EQ( store.elementType( "coordinates" ), RiaKeyValueStoreElementType::FLOAT64 );
    EXPECT_EQ( RiaKeyValueStoreUtil::convertToVector<double>( store.get( "coordinates", RiaKeyValueStoreElementType::FLOAT64 ) ), coordinates );

    // Reading with the wrong element type fails
    EXPECT_FALSE( store.get( "coordinates", RiaKeyValueStoreElementType::FLOAT32 ).has_value() );
    EXPECT_FALSE( store.get( "coordinates", RiaKeyValueStoreElementType::INT64 ).has_value() );
    EXPECT_FALSE( store.get( "missing", RiaKeyValueStoreElementType::FLOAT64 ).has_value() );

    // Values set without an element type are float32
    store.set( "coordinates", RiaKeyValueStoreUtil::convertToByteVector( std::vector<float>{ 1.0f } ) );
    EXPECT_EQ( store.elementType( "coordinates" ), RiaKeyValueStoreElementType::FLOAT32 );
    EXPECT_TRUE( store.get( "coordinates", RiaKeyValueStoreElementType::FLOAT32 ).has_value() );

    EXPECT_TRUE( store.remove( "coordinates" ) );
    EXPECT_FALSE( store.elementType( "coordinates" ).has_value() );

    store.set( "indices", RiaKeyValueStoreUtil::convertToByteVector( std::vector<uint32_t>{ 0, 1, 2 } ), RiaKeyValueStoreElementType::UINT32 );
    store.clear();
    EXPECT_FALSE( store.elementType( "indices" ).has_value() );
}
