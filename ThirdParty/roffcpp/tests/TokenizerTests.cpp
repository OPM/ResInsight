
#include "gtest/gtest.h"

#include <string>

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
