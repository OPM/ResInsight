
#include "gtest/gtest.h"

#include <fstream>
#include <string>

#include "RoffTestDataDirectory.hpp"
#include "Tokenizer.hpp"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( TokenizerTests, testTokenizeComment )
{
    std::stringstream stream( "#This is a comment.#" );
    Token             token = Tokenizer::tokenizeComment( stream );
    ASSERT_EQ( Token::Kind::COMMENT, token.kind() );
    ASSERT_EQ( 0, token.start() );
    ASSERT_EQ( 20, token.end() );
    ASSERT_EQ( 20, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( TokenizerTests, testTokenizeCommentWithoutStartTag )
{
    std::stringstream stream( "This comment did not have a start comment tag#" );
    ASSERT_THROW( Tokenizer::tokenizeComment( stream ), std::runtime_error );
    ASSERT_EQ( 0, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( TokenizerTests, testTokenizeCommentWithoutEndTag )
{
    std::stringstream stream( "#This is an incomplete comment." );
    ASSERT_THROW( Tokenizer::tokenizeComment( stream ), std::runtime_error );
    ASSERT_EQ( -1, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( TokenizerTests, testTokenizeSpace )
{
    std::stringstream stream( "     This is a string with leading spaces" );
    Tokenizer::tokenizeSpace( stream );
    // No token is produced, but stream position is moved to first non-space
    ASSERT_EQ( 5, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( TokenizerTests, testTokenizeNonSpaceAsSpace )
{
    std::stringstream stream( "not-really-space" );
    ASSERT_THROW( Tokenizer::tokenizeSpace( stream ), std::runtime_error );
    ASSERT_EQ( 0, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( TokenizerTests, testTokenizeDelimiterSpace )
{
    std::stringstream stream( "   s" );
    Tokenizer::tokenizeDelimiter( stream );
    ASSERT_EQ( 3, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( TokenizerTests, testTokenizeDelimiterComment )
{
    std::stringstream stream( "   #comment here before more space#    s" );
    Tokenizer::tokenizeDelimiter( stream );
    ASSERT_EQ( 39, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( TokenizerTests, testTokenizeAsciiString )
{
    std::stringstream stream( "\"string\"" );
    Token             token = Tokenizer::tokenizeString( stream );
    ASSERT_EQ( Token::Kind::STRING_LITERAL, token.kind() );
    ASSERT_EQ( 1, token.start() );
    ASSERT_EQ( 7, token.end() );
    ASSERT_EQ( 8, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( TokenizerTests, testTokenizeAsciiNumericValuesInt )
{
    std::stringstream stream( "123 " );
    Token             token = Tokenizer::tokenizeAsciiNumber( stream );
    ASSERT_EQ( Token::Kind::NUMERIC_VALUE, token.kind() );
    ASSERT_EQ( 0, token.start() );
    ASSERT_EQ( 3, token.end() );
    ASSERT_EQ( 3, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( TokenizerTests, testTokenizeAsciiNumericValuesDouble )
{
    std::stringstream stream( "1.0 " );
    Token             token = Tokenizer::tokenizeAsciiNumber( stream );
    ASSERT_EQ( Token::Kind::NUMERIC_VALUE, token.kind() );
    ASSERT_EQ( 0, token.start() );
    ASSERT_EQ( 3, token.end() );
    ASSERT_EQ( 3, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( TokenizerTests, testTokenizeName )
{
    std::stringstream stream( "roffeloff" );
    Token             token = Tokenizer::tokenizeName( stream );
    ASSERT_EQ( Token::Kind::NAME, token.kind() );
    ASSERT_EQ( 0, token.start() );
    ASSERT_EQ( 9, token.end() );
    ASSERT_EQ( -1, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( TokenizerTests, testTokenizeKeywordChar )
{
    std::stringstream stream( "char" );
    Token             token = Tokenizer::tokenizeKeyword( stream );
    ASSERT_EQ( Token::Kind::CHAR, token.kind() );
    ASSERT_EQ( 0, token.start() );
    ASSERT_EQ( 4, token.end() );
    ASSERT_EQ( -1, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( TokenizerTests, testTokenizeKeywordRoffBin )
{
    std::stringstream stream( "roff-bin" );
    Token             token = Tokenizer::tokenizeKeyword( stream );
    ASSERT_EQ( Token::Kind::ROFF_BIN, token.kind() );
    ASSERT_EQ( 0, token.start() );
    ASSERT_EQ( 8, token.end() );
    ASSERT_EQ( -1, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( TokenizerTests, testTokenizeAsciiTagKeyInt )
{
    std::stringstream  stream( "int x 3" );
    std::vector<Token> tokens = Tokenizer::tokenizeAsciiTagKey( stream );

    ASSERT_EQ( 3, tokens.size() );
    ASSERT_EQ( Token::Kind::INT, tokens[0].kind() );
    ASSERT_EQ( Token::Kind::NAME, tokens[1].kind() );
    ASSERT_EQ( Token::Kind::NUMERIC_VALUE, tokens[2].kind() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( TokenizerTests, testTokenizeAsciiTagKeyFloat )
{
    std::stringstream  stream( "float z 44.5" );
    std::vector<Token> tokens = Tokenizer::tokenizeAsciiTagKey( stream );

    ASSERT_EQ( 3, tokens.size() );
    ASSERT_EQ( Token::Kind::FLOAT, tokens[0].kind() );
    ASSERT_EQ( Token::Kind::NAME, tokens[1].kind() );
    ASSERT_EQ( Token::Kind::NUMERIC_VALUE, tokens[2].kind() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( TokenizerTests, testTokenizeTagGroup )
{
    std::stringstream  stream( "tag tagname bool a 1 int b 3 endtag" );
    std::vector<Token> tokens = Tokenizer::tokenizeTagGroup( stream );

    ASSERT_EQ( 9, tokens.size() );
    ASSERT_EQ( Token::Kind::TAG, tokens[0].kind() );
    ASSERT_EQ( Token::Kind::NAME, tokens[1].kind() );

    ASSERT_EQ( Token::Kind::BOOL, tokens[2].kind() );
    ASSERT_EQ( Token::Kind::NAME, tokens[3].kind() );
    ASSERT_EQ( Token::Kind::NUMERIC_VALUE, tokens[4].kind() );

    ASSERT_EQ( Token::Kind::INT, tokens[5].kind() );
    ASSERT_EQ( Token::Kind::NAME, tokens[6].kind() );
    ASSERT_EQ( Token::Kind::NUMERIC_VALUE, tokens[7].kind() );

    ASSERT_EQ( Token::Kind::ENDTAG, tokens[8].kind() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( TokenizerTests, testTokenizeArrayAsciiTagKeyInt )
{
    std::stringstream  stream( "array int my_array 6 1 2 3 4 5 6" );
    std::vector<Token> tokens = Tokenizer::tokenizeArrayAsciiTagKey( stream );

    ASSERT_EQ( 10, tokens.size() );
    ASSERT_EQ( Token::Kind::ARRAY, tokens[0].kind() );
    ASSERT_EQ( Token::Kind::INT, tokens[1].kind() );
    ASSERT_EQ( Token::Kind::NAME, tokens[2].kind() );
    ASSERT_EQ( Token::Kind::NUMERIC_VALUE, tokens[3].kind() );
    ASSERT_EQ( Token::Kind::NUMERIC_VALUE, tokens[4].kind() );
    ASSERT_EQ( Token::Kind::NUMERIC_VALUE, tokens[5].kind() );
    ASSERT_EQ( Token::Kind::NUMERIC_VALUE, tokens[6].kind() );
    ASSERT_EQ( Token::Kind::NUMERIC_VALUE, tokens[7].kind() );
    ASSERT_EQ( Token::Kind::NUMERIC_VALUE, tokens[8].kind() );
    ASSERT_EQ( Token::Kind::NUMERIC_VALUE, tokens[9].kind() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( TokenizerTests, testTokenizeArrayAsciiTagKeyStrings )
{
    std::stringstream  stream( "array char my_array 3 \"a\" \"bb\" \"ccc\"" );
    std::vector<Token> tokens = Tokenizer::tokenizeArrayAsciiTagKey( stream );

    ASSERT_EQ( 7, tokens.size() );
    ASSERT_EQ( Token::Kind::ARRAY, tokens[0].kind() );
    ASSERT_EQ( Token::Kind::CHAR, tokens[1].kind() );
    ASSERT_EQ( Token::Kind::NAME, tokens[2].kind() );
    ASSERT_EQ( Token::Kind::NUMERIC_VALUE, tokens[3].kind() );
    ASSERT_EQ( Token::Kind::STRING_LITERAL, tokens[4].kind() );
    ASSERT_EQ( Token::Kind::STRING_LITERAL, tokens[5].kind() );
    ASSERT_EQ( Token::Kind::STRING_LITERAL, tokens[6].kind() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( TokenizerTests, testTokenizeExampleFile )
{
    std::ifstream stream( std::string( TEST_DATA_DIR ) + "/facies_info.roff" );
    ASSERT_TRUE( stream.good() );

    std::vector<Token> tokens = Tokenizer::tokenizeStream( stream );
    ASSERT_EQ( 83, tokens.size() );

    auto readValueForToken = []( std::istream& stream, const Token& token ) {
        stream.seekg( token.start() );
        int         length = token.end() - token.start();
        std::string res;
        res.resize( length );
        stream.read( const_cast<char*>( res.data() ), length );
        return res;
    };

    stream.clear();
    stream.seekg( 0 );
    ASSERT_TRUE( stream.good() );

    ASSERT_EQ( "byteswaptest", readValueForToken( stream, tokens[4] ) );
    ASSERT_EQ( "codeNames", readValueForToken( stream, tokens[53] ) );
}
