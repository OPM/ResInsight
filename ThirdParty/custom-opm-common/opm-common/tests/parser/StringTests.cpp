#define BOOST_TEST_MODULE StringTests

#include <boost/test/unit_test.hpp>

#include <opm/common/utility/String.hpp>

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
    std::string_view view( src );


    std::string dst = "string";
    uppercase( view, dst );
    BOOST_CHECK_EQUAL( dst, "STRING" );
    BOOST_CHECK( view == "string" );
}

BOOST_AUTO_TEST_CASE( write_parts_of_dst ) {
    std::string src = "string";
    std::string_view view( src );


    std::string dst = "stringmixed";
    uppercase( view, dst );
    BOOST_CHECK_EQUAL( dst, "STRINGmixed" );
    BOOST_CHECK( view == "string" );
}



BOOST_AUTO_TEST_CASE(trim) {
    std::string s1 = "ABC";
    std::string s2 = " ABC";
    std::string s3 = "ABC ";
    std::string s4 = " ABC ";
    std::string s5 = "";
    std::string s6 = "      ";

    BOOST_CHECK_EQUAL(trim_copy(s1) , s1);
    BOOST_CHECK_EQUAL(trim_copy(s2) , s1);
    BOOST_CHECK_EQUAL(trim_copy(s3) , s1);
    BOOST_CHECK_EQUAL(trim_copy(s4) , s1);
    BOOST_CHECK_EQUAL(trim_copy(s5) , s5);
    BOOST_CHECK_EQUAL(trim_copy(s6) , s5);

    BOOST_CHECK_EQUAL(ltrim_copy(s1) , s1);
    BOOST_CHECK_EQUAL(ltrim_copy(s2) , s1);
    BOOST_CHECK_EQUAL(ltrim_copy(s3) , s3);
    BOOST_CHECK_EQUAL(ltrim_copy(s4) , s3);
    BOOST_CHECK_EQUAL(ltrim_copy(s5) , s5);
    BOOST_CHECK_EQUAL(ltrim_copy(s6) , s5);

    BOOST_CHECK_EQUAL(rtrim_copy(s1) , s1);
    BOOST_CHECK_EQUAL(rtrim_copy(s2) , s2);
    BOOST_CHECK_EQUAL(rtrim_copy(s3) , s1);
    BOOST_CHECK_EQUAL(rtrim_copy(s4) , s2);
    BOOST_CHECK_EQUAL(rtrim_copy(s5) , s5);
    BOOST_CHECK_EQUAL(rtrim_copy(s6) , s5);
}


BOOST_AUTO_TEST_CASE(replace_all) {
    std::string s1 = "lorem ipsum";

    replaceAll<std::string>(s1, "m", "foo");
    BOOST_CHECK_EQUAL(s1, "lorefoo ipsufoo");
}


BOOST_AUTO_TEST_CASE(split) {
    std::string s1 = "lorem ipsum";

    auto split1 = split_string(s1, ' ');
    BOOST_CHECK_EQUAL(split1.size(), 2);
    BOOST_CHECK_EQUAL(split1[0], "lorem");
    BOOST_CHECK_EQUAL(split1[1], "ipsum");

    auto split2 = split_string(s1, "r ");
    BOOST_CHECK_EQUAL(split2.size(), 3);
    BOOST_CHECK_EQUAL(split2[0], "lo");
    BOOST_CHECK_EQUAL(split2[1], "em");
    BOOST_CHECK_EQUAL(split2[2], "ipsum");

    auto split3 = split_string(s1, "m ");
    BOOST_CHECK_EQUAL(split3.size(), 2);
    BOOST_CHECK_EQUAL(split3[0], "lore");
    BOOST_CHECK_EQUAL(split3[1], "ipsu");
}
