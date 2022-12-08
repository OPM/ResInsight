
#include "gtest/gtest.h"

#include <fstream>
#include <string>

#include "AsciiTokenizer.hpp"
#include "RoffTestDataDirectory.hpp"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( AsciiTokenizerTests, testTokenizeComment )
{
    std::stringstream stream( "#This is a comment.#" );
    AsciiTokenizer    tokenizer;
    ASSERT_TRUE( tokenizer.tokenizeComment( stream ) );
    ASSERT_EQ( 20, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( AsciiTokenizerTests, testTokenizeCommentWithoutStartTag )
{
    std::stringstream stream( "This comment did not have a start comment tag#" );
    AsciiTokenizer    tokenizer;
    ASSERT_FALSE( tokenizer.tokenizeComment( stream ) );
    ASSERT_EQ( 0, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( AsciiTokenizerTests, testTokenizeCommentWithoutEndTag )
{
    std::stringstream stream( "#This is an incomplete comment." );
    AsciiTokenizer    tokenizer;
    ASSERT_FALSE( tokenizer.tokenizeComment( stream ) );
    ASSERT_EQ( -1, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( AsciiTokenizerTests, testTokenizeSpace )
{
    std::stringstream stream( "     This is a string with leading spaces" );
    AsciiTokenizer    tokenizer;
    tokenizer.tokenizeSpace( stream );
    // No token is produced, but stream position is moved to first non-space
    ASSERT_EQ( 5, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( AsciiTokenizerTests, testTokenizeNonSpaceAsSpace )
{
    std::stringstream stream( "not-really-space" );
    AsciiTokenizer    tokenizer;
    ASSERT_FALSE( tokenizer.tokenizeSpace( stream ) );
    ASSERT_EQ( 0, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( AsciiTokenizerTests, testTokenizeDelimiterSpace )
{
    std::stringstream stream( "   s" );
    AsciiTokenizer    tokenizer;
    tokenizer.tokenizeDelimiter( stream );
    ASSERT_EQ( 3, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( AsciiTokenizerTests, testTokenizeDelimiterComment )
{
    std::stringstream stream( "   #comment here before more space#    s" );
    AsciiTokenizer    tokenizer;
    tokenizer.tokenizeDelimiter( stream );
    ASSERT_EQ( 39, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( AsciiTokenizerTests, testTokenizeAsciiString )
{
    std::stringstream stream( "\"string\"" );
    AsciiTokenizer    tokenizer;
    Token             token = tokenizer.tokenizeString( stream ).value();
    ASSERT_EQ( Token::Kind::STRING_LITERAL, token.kind() );
    ASSERT_EQ( 1u, token.start() );
    ASSERT_EQ( 7u, token.end() );
    ASSERT_EQ( 8, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( AsciiTokenizerTests, testTokenizeAsciiNumericValuesInt )
{
    std::stringstream stream( "123 " );
    AsciiTokenizer    tokenizer;
    Token             token = tokenizer.tokenizeNumber( stream ).value();
    ASSERT_EQ( Token::Kind::NUMERIC_VALUE, token.kind() );
    ASSERT_EQ( 0u, token.start() );
    ASSERT_EQ( 3u, token.end() );
    ASSERT_EQ( 3, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( AsciiTokenizerTests, testTokenizeAsciiNumericValuesDouble )
{
    std::stringstream stream( "1.0 " );
    AsciiTokenizer    tokenizer;
    Token             token = tokenizer.tokenizeNumber( stream ).value();
    ASSERT_EQ( Token::Kind::NUMERIC_VALUE, token.kind() );
    ASSERT_EQ( 0u, token.start() );
    ASSERT_EQ( 3u, token.end() );
    ASSERT_EQ( 3, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( AsciiTokenizerTests, testTokenizeName )
{
    std::stringstream stream( "roffeloff" );
    AsciiTokenizer    tokenizer;
    Token             token = tokenizer.tokenizeName( stream );
    ASSERT_EQ( Token::Kind::NAME, token.kind() );
    ASSERT_EQ( 0u, token.start() );
    ASSERT_EQ( 9u, token.end() );
    ASSERT_EQ( -1, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( AsciiTokenizerTests, testTokenizeKeywordChar )
{
    std::stringstream stream( "char" );
    AsciiTokenizer    tokenizer;
    Token             token = tokenizer.tokenizeKeyword( stream );
    ASSERT_EQ( Token::Kind::CHAR, token.kind() );
    ASSERT_EQ( 0u, token.start() );
    ASSERT_EQ( 4u, token.end() );
    ASSERT_EQ( -1, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( AsciiTokenizerTests, testTokenizeKeywordRoffBin )
{
    std::stringstream stream( "roff-bin" );
    AsciiTokenizer    tokenizer;
    Token             token = tokenizer.tokenizeKeyword( stream );
    ASSERT_EQ( Token::Kind::ROFF_BIN, token.kind() );
    ASSERT_EQ( 0u, token.start() );
    ASSERT_EQ( 8u, token.end() );
    ASSERT_EQ( -1, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( AsciiTokenizerTests, testTokenizeTagKeyInt )
{
    std::stringstream  stream( "int x 3" );
    AsciiTokenizer     tokenizer;
    std::vector<Token> tokens = tokenizer.tokenizeTagKey( stream );

    ASSERT_EQ( 3u, tokens.size() );
    ASSERT_EQ( Token::Kind::INT, tokens[0].kind() );
    ASSERT_EQ( Token::Kind::NAME, tokens[1].kind() );
    ASSERT_EQ( Token::Kind::NUMERIC_VALUE, tokens[2].kind() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( AsciiTokenizerTests, testTokenizeTagKeyFloat )
{
    std::stringstream  stream( "float z 44.5" );
    AsciiTokenizer     tokenizer;
    std::vector<Token> tokens = tokenizer.tokenizeTagKey( stream );

    ASSERT_EQ( 3u, tokens.size() );
    ASSERT_EQ( Token::Kind::FLOAT, tokens[0].kind() );
    ASSERT_EQ( Token::Kind::NAME, tokens[1].kind() );
    ASSERT_EQ( Token::Kind::NUMERIC_VALUE, tokens[2].kind() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( AsciiTokenizerTests, testTokenizeTagGroupFloatWithMoreSpecialChars )
{
    std::stringstream  stream( "tag some_name float x -1.00000000E+00 float z 1.3e-1 endtag" );
    AsciiTokenizer     tokenizer;
    std::vector<Token> tokens = tokenizer.tokenizeTagGroup( stream );

    ASSERT_EQ( 9u, tokens.size() );
    ASSERT_EQ( Token::Kind::TAG, tokens[0].kind() );
    ASSERT_EQ( Token::Kind::NAME, tokens[1].kind() );

    ASSERT_EQ( Token::Kind::FLOAT, tokens[2].kind() );
    ASSERT_EQ( Token::Kind::NAME, tokens[3].kind() );
    ASSERT_EQ( Token::Kind::NUMERIC_VALUE, tokens[4].kind() );

    ASSERT_EQ( Token::Kind::FLOAT, tokens[5].kind() );
    ASSERT_EQ( Token::Kind::NAME, tokens[6].kind() );
    ASSERT_EQ( Token::Kind::NUMERIC_VALUE, tokens[7].kind() );

    ASSERT_EQ( Token::Kind::ENDTAG, tokens[8].kind() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( AsciiTokenizerTests, testTokenizeTagGroup )
{
    std::stringstream  stream( "tag tagname bool a 1 int b 3 endtag" );
    AsciiTokenizer     tokenizer;
    std::vector<Token> tokens = tokenizer.tokenizeTagGroup( stream );

    ASSERT_EQ( 9u, tokens.size() );
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
TEST( AsciiTokenizerTests, testTokenizeArrayTagKeyInt )
{
    std::stringstream  stream( "array int my_array 6 1 2 3 4 5 6" );
    AsciiTokenizer     tokenizer;
    std::vector<Token> tokens = tokenizer.tokenizeArrayTagKey( stream );

    ASSERT_EQ( 10u, tokens.size() );
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
TEST( AsciiTokenizerTests, testTokenizeArrayTagKeyStrings )
{
    std::stringstream  stream( "array char my_array 3 \"a\" \"bb\" \"ccc\"" );
    AsciiTokenizer     tokenizer;
    std::vector<Token> tokens = tokenizer.tokenizeArrayTagKey( stream );

    ASSERT_EQ( 7u, tokens.size() );
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
TEST( AsciiTokenizerTests, testTokenizeExampleFile )
{
    std::ifstream stream( std::string( TEST_DATA_DIR ) + "/facies_info.roff" );
    ASSERT_TRUE( stream.good() );

    AsciiTokenizer     tokenizer;
    std::vector<Token> tokens = tokenizer.tokenizeStream( stream );
    ASSERT_EQ( 109u, tokens.size() );

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
