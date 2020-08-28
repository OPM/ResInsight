#define BOOST_TEST_MODULE StringTests

#include <boost/test/unit_test.hpp>

#include <opm/common/utility/String.hpp>
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

BOOST_AUTO_TEST_CASE(fullStringView) {
    std::string srcstr = "lorem ipsum";
    string_view view( srcstr );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            srcstr.begin(), srcstr.end(),
            view.begin(), view.end() );
}

BOOST_AUTO_TEST_CASE(viewCorrectSize) {
    std::string srcstr = "lorem ipsum";

    string_view full( srcstr );
    BOOST_CHECK_EQUAL( srcstr.size(), full.size() );

    string_view view( srcstr, 5 );
    BOOST_CHECK_EQUAL( 5, view.size() );
    BOOST_CHECK_EQUAL( 5, view.length() );
}

BOOST_AUTO_TEST_CASE(viewOperatorAt) {
    std::string srcstr = "lorem ipsum";
    string_view view( srcstr );

    for( size_t i = 0; i < view.size(); ++i )
        BOOST_CHECK_EQUAL( view[ i ], srcstr[ i ] );
}

BOOST_AUTO_TEST_CASE(viewFrontBack) {
    std::string srcstr = "lorem ipsum";
    string_view view( srcstr );

    BOOST_CHECK_EQUAL( view.front(), 'l' );
    BOOST_CHECK_EQUAL( view.back(), 'm' );
}


BOOST_AUTO_TEST_CASE(viewSubstr) {
    std::string srcstr = "lorem ipsum";
    string_view view( srcstr );

    BOOST_CHECK_NO_THROW( view.string() );
    BOOST_CHECK_EQUAL( srcstr, view.string() );
    BOOST_CHECK_EQUAL( srcstr, view.substr() );
    BOOST_CHECK_EQUAL( "", view.substr( 0, 0 ) );

    BOOST_CHECK_EQUAL( srcstr.substr( 1 ), view.substr( 1 ) );
    BOOST_CHECK_EQUAL( srcstr, view.substr( 0, srcstr.size() + 1 ));
    BOOST_CHECK_EQUAL( "", view.substr( 1, 0 ));
    BOOST_CHECK_EQUAL( "", view.substr( 0, 0 ) );

    BOOST_CHECK_THROW( view.substr( srcstr.size() + 1 ), std::out_of_range );
}

BOOST_AUTO_TEST_CASE(viewStream) {
    std::string srcstr = "lorem ipsum";
    string_view view( srcstr );

    std::stringstream str;
    str << view;

    BOOST_CHECK_EQUAL( srcstr, str.str() );
}

BOOST_AUTO_TEST_CASE(equalityOperators) {
    std::string srcstr = "lorem ipsum";
    std::string diffstr = "lorem";
    string_view view( srcstr );

    BOOST_CHECK_EQUAL( srcstr, view );
    BOOST_CHECK_NE( diffstr, view );

    BOOST_CHECK_EQUAL( view, srcstr );
    BOOST_CHECK_NE( view, diffstr );

    BOOST_CHECK_EQUAL( "lorem ipsum", view );
    BOOST_CHECK_NE( "lorem", view );

    BOOST_CHECK_EQUAL( view, "lorem ipsum" );
    BOOST_CHECK_NE( view, "lorem" );
}

BOOST_AUTO_TEST_CASE(plusOperator) {
    std::string total = "lorem ipsum";
    std::string lhs = "lorem";
    std::string ws = " ";
    std::string rhs = "ipsum";

    string_view lhs_view( lhs );
    string_view rhs_view( rhs );

    BOOST_CHECK_EQUAL( total, lhs_view + ws + rhs_view );
    BOOST_CHECK_EQUAL( lhs + ws, lhs_view + ws );
    BOOST_CHECK_EQUAL( ws + rhs, ws + rhs_view );
}



BOOST_AUTO_TEST_CASE(strncmp_function) {
    std::string s = "A BB CCC DDDD";
    string_view view(s);

    BOOST_CHECK(!view.starts_with("this is a very long string -longer than the view"));
    BOOST_CHECK(view.starts_with("A"));
    BOOST_CHECK(view.starts_with("A BB"));
    BOOST_CHECK(!view.starts_with("A BB D"));
    BOOST_CHECK(view.starts_with(s));

    BOOST_CHECK_EQUAL( view.find("A"), 0);
    BOOST_CHECK_EQUAL( view.find("BB"), 2);
    BOOST_CHECK_EQUAL( view.find("C"), 5);
    BOOST_CHECK_EQUAL( view.find("CCCC"), std::string::npos);
    BOOST_CHECK_EQUAL( view.find("DDDD"), 9);

    BOOST_CHECK_EQUAL( view.find('A'), 0);
    BOOST_CHECK_EQUAL( view.find('B'), 2);
    BOOST_CHECK_EQUAL( view.find('C'), 5);
    BOOST_CHECK_EQUAL( view.find('X'), std::string::npos);
    BOOST_CHECK_EQUAL( view.find('D'), 9);
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
