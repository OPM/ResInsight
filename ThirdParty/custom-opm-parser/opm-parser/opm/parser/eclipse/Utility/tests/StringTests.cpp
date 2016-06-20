#define BOOST_TEST_MODULE StringTests

#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>

#include <opm/parser/eclipse/Utility/String.hpp>
#include <opm/parser/eclipse/Utility/Stringview.hpp>

using namespace Opm;

BOOST_AUTO_TEST_CASE( uppercase_copy ) {
    const std::string src = "string";
    const std::string dst = uppercase( src );

    BOOST_CHECK_EQUAL( src, "string" );
    BOOST_CHECK_EQUAL( dst, "STRING" );
}

BOOST_AUTO_TEST_CASE( uppercase_inplace ) {
    std::string src = "string";
    auto& ref = uppercase( src, src );

    BOOST_CHECK_EQUAL( src, "STRING" );
    BOOST_CHECK_EQUAL( src, ref );
    BOOST_CHECK_EQUAL( std::addressof( src ), std::addressof( ref ) );
}

BOOST_AUTO_TEST_CASE( nonconst_ref ) {
    std::string src = "string";
    auto dst = uppercase( src );

    BOOST_CHECK_EQUAL( src, "string" );
    BOOST_CHECK_EQUAL( dst, "STRING" );
}

BOOST_AUTO_TEST_CASE( uppercase_move ) {
    std::string src = "string";
    auto dst = uppercase( std::move( src ) );

    BOOST_CHECK_EQUAL( dst, "STRING" );
}

BOOST_AUTO_TEST_CASE( uppercase_mixed_type ) {
    std::string src = "string";
    string_view view( src );


    std::string dst = "string";
    uppercase( view, dst );
    BOOST_CHECK_EQUAL( dst, "STRING" );
    BOOST_CHECK_EQUAL( view, "string" );
}

BOOST_AUTO_TEST_CASE( write_parts_of_dst ) {
    std::string src = "string";
    string_view view( src );


    std::string dst = "stringmixed";
    uppercase( view, dst );
    BOOST_CHECK_EQUAL( dst, "STRINGmixed" );
    BOOST_CHECK_EQUAL( view, "string" );
}
