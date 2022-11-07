
#include "gtest/gtest.h"

#include <fstream>
#include <string>
#include <variant>

#include "Parser.hpp"
#include "RoffTestDataDirectory.hpp"
#include "Token.hpp"
#include "Tokenizer.hpp"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( ParserTests, testTokenizeExampleFile )
{
    std::ifstream stream( std::string( TEST_DATA_DIR ) + "/facies_info.roff" );
    ASSERT_TRUE( stream.good() );

    std::vector<Token> tokens = Tokenizer::tokenizeStream( stream );
    Parser             parser( stream, tokens );

    std::vector<std::pair<std::string, RoffScalar>> values = parser.scalarNamedValues();
    ASSERT_FALSE( values.empty() );

    auto hasValue = []( auto values, const std::string& keyword ) {
        return std::any_of( values.cbegin(), values.cend(), [&keyword]( const auto& arg ) {
            return arg.first == keyword;
        } );
    };

    auto getValue = []( const std::vector<std::pair<std::string, RoffScalar>>& values, const std::string& keyword ) {
        auto a =
            std::find_if( values.begin(), values.end(), [&keyword]( const auto& arg ) { return arg.first == keyword; } );
        if ( a != values.end() )
            return a->second;
        else
            return RoffScalar( std::numeric_limits<float>::infinity() );
    };

    ASSERT_TRUE( hasValue( values, "filedata.byteswaptest" ) );
    ASSERT_EQ( 1, std::get<int>( getValue( values, "filedata.byteswaptest" ) ) );

    ASSERT_TRUE( hasValue( values, "version.major" ) );
    ASSERT_EQ( 2, std::get<int>( getValue( values, "version.major" ) ) );

    ASSERT_TRUE( hasValue( values, "dimensions.nX" ) );
    ASSERT_EQ( 46, std::get<int>( getValue( values, "dimensions.nX" ) ) );

    ASSERT_TRUE( hasValue( values, "parameter.name" ) );
    ASSERT_EQ( std::string( "composite" ), std::get<std::string>( getValue( values, "parameter.name" ) ) );

    ASSERT_TRUE( hasValue( values, "parameter.fl" ) );
    ASSERT_DOUBLE_EQ( 1.23f, std::get<float>( getValue( values, "parameter.fl" ) ) );

    ASSERT_TRUE( hasValue( values, "parameter.db" ) );
    ASSERT_DOUBLE_EQ( 9000.9, std::get<double>( getValue( values, "parameter.db" ) ) );

    ASSERT_TRUE( hasValue( values, "parameter.bt" ) );
    ASSERT_EQ( 42, std::get<unsigned char>( getValue( values, "parameter.bt" ) ) );

    ASSERT_TRUE( hasValue( values, "parameter.bo" ) );
    ASSERT_EQ( true, std::get<bool>( getValue( values, "parameter.bo" ) ) );
}
