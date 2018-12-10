#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include <catch/catch.hpp>

#include <ecl/fortio.h>

#include "matchers.hpp"

namespace {

struct fcloser {
    void operator()( std::FILE* fp ) { fclose( fp ); }
};

using ufile = std::unique_ptr< std::FILE, fcloser >;

}

using Catch::Matchers::Equals;

TEST_CASE("records with broken tail can be read", "[fortio][f77]") {
    ufile handle( std::tmpfile() );
    REQUIRE( handle );
    auto* fp = handle.get();

    std::vector< std::int32_t > src( 10, 0 );
    const std::int32_t head = sizeof( std::int32_t ) * src.size();

    for( int i = 0; i < int(src.size()); ++i )
        src[ i ] = i;

    auto elems = std::fwrite( &head, sizeof( head ), 1, fp );
    REQUIRE_THAT( elems, FwriteCount( 1 ) );

    elems = std::fwrite( src.data(), sizeof( head ), src.size(), fp );
    REQUIRE_THAT( elems, FwriteCount( src.size() ) );

    std::int32_t size = src.size();

    for( bool write_tail : { true, false } ) SECTION(
          write_tail
        ? "tail is valid, but mismatch with head"
        : "tail is missing" ) {

        if( write_tail ) {
            auto tail = head + 1;
            auto elems = std::fwrite( &tail, sizeof( head ), 1, fp );
            REQUIRE_THAT( elems, FwriteCount( 1 ) );
        }

        SECTION("querying size is not affected") {
            std::rewind( fp );

            Err err = eclfio_sizeof( fp, "e", &size );
            CHECK( err == Err::ok() );
            CHECK( size == 10 );
        }

        SECTION("failure with strict read") {
            std::rewind( fp );
            auto out = src;
            const auto pos = std::ftell( fp );

            Err err = eclfio_get( fp, "e", &size, out.data() );
            CHECK( err == Err::invalid_record() );
            CHECK( size == 10 );
            CHECK( pos == std::ftell( fp ) );
        }

        SECTION("success with allow-notail ($)") {
            std::rewind( fp );
            auto out = src;
            const auto pos = std::ftell( fp );

            Err err = eclfio_get( fp, "e$", &size, out.data() );
            CHECK( err == Err::ok() );
            CHECK( size == 10 );
            CHECK( pos < std::ftell( fp ) );
            CHECK_THAT( out, Equals( src ) );
        }

        SECTION("success with force-notail (~)") {
            std::rewind( fp );
            auto out = src;
            const auto pos = std::ftell( fp );

            Err err = eclfio_get( fp, "e~", &size, out.data() );
            CHECK( err == Err::ok() );
            CHECK( size == 10 );
            CHECK( pos < std::ftell( fp ) );
            CHECK_THAT( out, Equals( src ) );
        }
    }
}

TEST_CASE("record with valid, but wrong head", "[fortio][f77]") {
    ufile handle( std::tmpfile() );
    REQUIRE( handle );
    auto* fp = handle.get();

    std::vector< std::int32_t > src( 10, 0 );
    const std::int32_t head = sizeof(std::int32_t) * (src.size() + 2) ;

    auto elems = std::fwrite( &head, sizeof( head ), 1, fp );
    REQUIRE_THAT( elems, FwriteCount( 1 ) );

    elems = std::fwrite( src.data(), sizeof( head ), src.size(), fp );
    REQUIRE_THAT( elems, FwriteCount( src.size() ) );

    std::rewind( fp );
    const auto pos = std::ftell( fp );
    std::vector< std::int32_t > out( head, 0 );
    std::int32_t size = out.size();
    Err err = eclfio_get( fp, "e", &size, out.data() );

    CHECK( err == Err::read() );
    CHECK( pos == std::ftell( fp ) );
    CHECK( size == out.size() );
}

TEST_CASE("record with invalid head", "[fortio][f77]") {
    ufile handle( std::tmpfile() );
    REQUIRE( handle );
    auto* fp = handle.get();

    std::vector< std::int32_t > src( 10, 0 );

    for( std::int32_t head : { -4, 11 } ) SECTION(
          head < 0
        ? "negative length"
        : "length not multiple of element size" ) {

        auto elems = std::fwrite( &head, sizeof( head ), 1, fp );
        REQUIRE_THAT( elems, FwriteCount( 1 ) );

        const std::vector< std::int32_t > src( 10, 0 );
        elems = std::fwrite( src.data(), sizeof( head ), src.size(), fp );
        REQUIRE_THAT( elems, FwriteCount( src.size() ) );

        std::rewind( fp );
        const auto pos = std::ftell( fp );
        std::int32_t size = src.size();
        Err err = eclfio_get( fp, "e", nullptr, nullptr );

        CHECK( err == Err::invalid_record() );
        CHECK( pos == std::ftell( fp ) );
        CHECK( size == src.size() );
    }
}
