
#include "gtest/gtest.h"

#include <fstream>
#include <string>

#include "BinaryTokenizer.hpp"
#include "RoffTestDataDirectory.hpp"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BinaryTokenizerTests, testTokenizeComment )
{
    char               bytes[] = { '#', 'c', 'o', 'm', 'm', 'e', 'n', 't', '#', '\0' };
    std::istringstream stream( std::string( std::begin( bytes ), std::end( bytes ) ) );
    stream.seekg( 0 );

    BinaryTokenizer tokenizer;
    ASSERT_TRUE( tokenizer.tokenizeComment( stream ) );
    ASSERT_EQ( 9, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BinaryTokenizerTests, testTokenizeCommentWithoutStartTag )
{
    char               bytes[] = { 'c', 'o', 'm', 'm', 'e', 'n', 't', '#', '\0' };
    std::istringstream stream( std::string( std::begin( bytes ), std::end( bytes ) ) );
    stream.seekg( 0 );

    BinaryTokenizer tokenizer;
    ASSERT_FALSE( tokenizer.tokenizeComment( stream ) );
    ASSERT_EQ( 0, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BinaryTokenizerTests, testTokenizeCommentWithoutEndTag )
{
    char               bytes[] = { '#', 'c', 'o', 'm', 'm', '\0' };
    std::istringstream stream( std::string( std::begin( bytes ), std::end( bytes ) ) );
    stream.seekg( 0 );

    BinaryTokenizer tokenizer;
    ASSERT_FALSE( tokenizer.tokenizeComment( stream ) );
    ASSERT_EQ( -1, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BinaryTokenizerTests, testTokenizeDelimiterSpace )
{
    char               bytes[] = { '\0', 'a', 'b' };
    std::istringstream stream( std::string( std::begin( bytes ), std::end( bytes ) ) );
    stream.seekg( 0 );

    BinaryTokenizer tokenizer;
    tokenizer.tokenizeDelimiter( stream );
    ASSERT_EQ( 1, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BinaryTokenizerTests, testTokenizeBinaryString )
{
    char               bytes[] = { 'a', 's', 't', 'r', 'i', 'n', 'g', '\0' };
    std::istringstream stream( std::string( std::begin( bytes ), std::end( bytes ) ) );
    stream.seekg( 0 );

    BinaryTokenizer tokenizer;
    Token           token = tokenizer.tokenizeString( stream ).value();
    ASSERT_EQ( Token::Kind::STRING_LITERAL, token.kind() );
    ASSERT_EQ( 0, token.start() );
    ASSERT_EQ( 7, token.end() );
    ASSERT_EQ( 8, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BinaryTokenizerTests, testTokenizeNumericValuesInt )
{
    char               bytes[] = { '0', '0', '0', '1', '\0' };
    std::istringstream stream( std::string( std::begin( bytes ), std::end( bytes ) ) );
    stream.seekg( 0 );

    BinaryTokenizer tokenizer;
    Token           token = tokenizer.tokenizeNumber( stream, Token::Kind::INT ).value();
    ASSERT_EQ( Token::Kind::BINARY_NUMERIC_VALUE, token.kind() );
    ASSERT_EQ( 0, token.start() );
    ASSERT_EQ( 4, token.end() );
    ASSERT_EQ( 4, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BinaryTokenizerTests, testTokenizeNumericValuesDouble )
{
    char               bytes[] = { '3', '4', '1', '1', '\0' };
    std::istringstream stream( std::string( std::begin( bytes ), std::end( bytes ) ) );
    stream.seekg( 0 );

    BinaryTokenizer tokenizer;
    Token           token = tokenizer.tokenizeNumber( stream, Token::Kind::FLOAT ).value();
    ASSERT_EQ( Token::Kind::BINARY_NUMERIC_VALUE, token.kind() );
    ASSERT_EQ( 0, token.start() );
    ASSERT_EQ( 4, token.end() );
    ASSERT_EQ( 4, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BinaryTokenizerTests, testTokenizeName )
{
    char               bytes[] = { '\0', 'a', 'b', 'c', '\0' };
    std::istringstream stream( std::string( std::begin( bytes ), std::end( bytes ) ) );
    stream.seekg( 0 );

    BinaryTokenizer tokenizer;
    Token           token = tokenizer.tokenizeName( stream );
    ASSERT_EQ( Token::Kind::NAME, token.kind() );
    ASSERT_EQ( 1, token.start() );
    ASSERT_EQ( 4, token.end() );
    ASSERT_EQ( 5, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BinaryTokenizerTests, testTokenizeKeywordChar )
{
    char               bytes[] = { '\0', 'c', 'h', 'a', 'r', '\0' };
    std::istringstream stream( std::string( std::begin( bytes ), std::end( bytes ) ) );
    stream.seekg( 0 );

    BinaryTokenizer tokenizer;
    Token           token = tokenizer.tokenizeKeyword( stream );
    ASSERT_EQ( Token::Kind::CHAR, token.kind() );
    ASSERT_EQ( 1, token.start() );
    ASSERT_EQ( 5, token.end() );
    ASSERT_EQ( 5, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BinaryTokenizerTests, testTokenizeKeywordRoffBin )
{
    char               bytes[] = { 'r', 'o', 'f', 'f', '-', 'b', 'i', 'n', '\0' };
    std::istringstream stream( std::string( std::begin( bytes ), std::end( bytes ) ) );
    stream.seekg( 0 );

    BinaryTokenizer tokenizer;
    Token           token = tokenizer.tokenizeKeyword( stream );
    ASSERT_EQ( Token::Kind::ROFF_BIN, token.kind() );
    ASSERT_EQ( 0, token.start() );
    ASSERT_EQ( 8, token.end() );
    ASSERT_EQ( 8, stream.tellg() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BinaryTokenizerTests, testTokenizeBinaryTagKeyInt )
{
    char               bytes[] = { 'i', 'n', 't', '\0', 'x', '\0', '0', '3', '0', '0', '\0' };
    std::istringstream stream( std::string( std::begin( bytes ), std::end( bytes ) ) );
    stream.seekg( 0 );

    BinaryTokenizer    tokenizer;
    std::vector<Token> tokens = tokenizer.tokenizeTagKey( stream );

    ASSERT_EQ( 3, tokens.size() );
    ASSERT_EQ( Token::Kind::INT, tokens[0].kind() );
    ASSERT_EQ( Token::Kind::NAME, tokens[1].kind() );
    ASSERT_EQ( Token::Kind::BINARY_NUMERIC_VALUE, tokens[2].kind() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BinaryTokenizerTests, testTokenizeTagGroup )
{
    char bytes[] = {
        't', 'a',  'g', '\0', 'x', '\0', 'b', 'o', 'o',  'l', '\0', 'a', '\0', '1', '\0', 'i',  'n',
        't', '\0', 'b', '\0', '0', '1',  '0', '0', '\0', 'e', 'n',  'd', 't',  'a', 'g',  '\0',
    };
    std::istringstream stream( std::string( std::begin( bytes ), std::end( bytes ) ) );
    stream.seekg( 0 );

    BinaryTokenizer    tokenizer;
    std::vector<Token> tokens = tokenizer.tokenizeTagGroup( stream );

    ASSERT_EQ( 9, tokens.size() );
    ASSERT_EQ( Token::Kind::TAG, tokens[0].kind() );
    ASSERT_EQ( Token::Kind::NAME, tokens[1].kind() );

    ASSERT_EQ( Token::Kind::BOOL, tokens[2].kind() );
    ASSERT_EQ( Token::Kind::NAME, tokens[3].kind() );
    ASSERT_EQ( Token::Kind::BINARY_NUMERIC_VALUE, tokens[4].kind() );

    ASSERT_EQ( Token::Kind::INT, tokens[5].kind() );
    ASSERT_EQ( Token::Kind::NAME, tokens[6].kind() );
    ASSERT_EQ( Token::Kind::BINARY_NUMERIC_VALUE, tokens[7].kind() );

    ASSERT_EQ( Token::Kind::ENDTAG, tokens[8].kind() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BinaryTokenizerTests, testTokenizeArrayBinaryTagKeyInt )
{
    char bytes[] = {
        'a',    'r',    'r',    'a',  'y', '\0', 'i', 'n', 't', '\0', 'x', '\0', '\x02',
        '\x00', '\x00', '\x00', '\0', '0', '0',  '0', '0', '0', '1',  '0', '0',
    };

    std::istringstream stream( std::string( std::begin( bytes ), std::end( bytes ) ) );
    stream.seekg( 0 );

    BinaryTokenizer    tokenizer;
    std::vector<Token> tokens = tokenizer.tokenizeArrayTagKey( stream );

    ASSERT_EQ( 5, tokens.size() );
    ASSERT_EQ( Token::Kind::ARRAY, tokens[0].kind() );
    ASSERT_EQ( 0, tokens[0].start() );
    ASSERT_EQ( 5, tokens[0].end() );

    ASSERT_EQ( Token::Kind::INT, tokens[1].kind() );
    ASSERT_EQ( 6, tokens[1].start() );
    ASSERT_EQ( 9, tokens[1].end() );

    ASSERT_EQ( Token::Kind::NAME, tokens[2].kind() );
    ASSERT_EQ( 10, tokens[2].start() );
    ASSERT_EQ( 11, tokens[2].end() );

    ASSERT_EQ( Token::Kind::BINARY_NUMERIC_VALUE, tokens[3].kind() );
    ASSERT_EQ( 12, tokens[3].start() );
    ASSERT_EQ( 16, tokens[3].end() );

    ASSERT_EQ( Token::Kind::ARRAYBLOB, tokens[4].kind() );
    ASSERT_EQ( 16, tokens[4].start() );
    ASSERT_EQ( 24, tokens[4].end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BinaryTokenizerTests, testTokenizeExampleFile )
{
    std::ifstream stream( std::string( TEST_DATA_DIR ) + "/facies_info.roffbin" );
    ASSERT_TRUE( stream.good() );

    BinaryTokenizer    tokenizer;
    std::vector<Token> tokens = tokenizer.tokenizeStream( stream );

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
    ASSERT_EQ( "floatData", readValueForToken( stream, tokens[63] ) );
}
