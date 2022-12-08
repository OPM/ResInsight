#include "BinaryTokenizer.hpp"

#include <cassert>
#include <cctype>
#include <optional>
#include <stdexcept>
#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
BinaryTokenizer::BinaryTokenizer()
    : Tokenizer()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
BinaryTokenizer::~BinaryTokenizer()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool BinaryTokenizer::tokenizeSpace( std::istream& stream )
{
    auto start    = stream.tellg();
    char readChar = static_cast<char>( stream.get() );
    if ( readChar != '\0' )
    {
        stream.seekg( start );
        return false;
    }
    else
    {
        return true;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<Token> BinaryTokenizer::tokenizeString( std::istream& stream )
{
    tokenizeDelimiter( stream );

    auto start    = stream.tellg();
    auto end      = start;
    char readChar = static_cast<char>( stream.get() );
    while ( stream.good() && readChar != '\0' )
    {
        end      = stream.tellg();
        readChar = static_cast<char>( stream.get() );
    }

    if ( !stream.good() )
    {
        // Reached unexpected end of file.
        stream.seekg( start );
        return {};
    }

    return Token( Token::Kind::STRING_LITERAL, start, end );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Token BinaryTokenizer::tokenizeName( std::istream& stream )
{
    auto t = tokenizeString( stream );
    if ( !t ) throw std::runtime_error( "Could not tokenize name." );

    return Token( Token::Kind::NAME, t.value().start(), t.value().end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<Token> BinaryTokenizer::tokenizeNumber( std::istream& stream, Token::Kind kind )
{
    auto start  = stream.tellg();
    int  length = Token::binaryTokenSizeInBytes( kind );
    auto end    = static_cast<size_t>( start ) + length;
    stream.seekg( end );
    return Token( Token::Kind::BINARY_NUMERIC_VALUE, start, end );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<Token> BinaryTokenizer::tokenizeValue( std::istream& stream, Token::Kind kind )
{
    if ( kind == Token::Kind::CHAR )
        return tokenizeString( stream );
    else
        return tokenizeNumber( stream, kind );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<Token> BinaryTokenizer::tokenizeWord( std::istream& stream, const std::string& keyword, Token::Kind kind )
{
    tokenizeDelimiter( stream );
    auto start = stream.tellg();

    int length = static_cast<int>( keyword.size() );

    std::vector<char> buffer( length );
    stream.read( &buffer[0], length );

    std::string word( buffer.begin(), buffer.end() );

    if ( stream && keyword.compare( word ) == 0 )
    {
        auto end = static_cast<size_t>( start ) + keyword.length();
        return Token( kind, start, end );
    }
    else
    {
        stream.clear();
        stream.seekg( start );
        return {};
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Token> BinaryTokenizer::tokenizeTagKeyInternal( std::istream& stream )
{
    std::vector<Token> tokens;
    Token              typeToken = tokenizeSimpleType( stream );
    tokens.push_back( typeToken );

    tokens.push_back( tokenizeName( stream ) );

    auto value = tokenizeValue( stream, typeToken.kind() );
    if ( value )
        tokens.push_back( value.value() );
    else
        throw std::runtime_error( "Invalid value." );

    return tokens;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Token> BinaryTokenizer::tokenizeArrayTagKey( std::istream& stream )
{
    std::vector<Token> tokens;
    tokens.push_back( tokenizeKeyword( stream, { std::make_pair( Token::Kind::ARRAY, "array" ) } ) );

    auto typeToken = tokenizeSimpleType( stream );
    tokens.push_back( typeToken );
    tokens.push_back( tokenizeName( stream ) );

    auto numElementsToken = tokenizeNumber( stream, Token::Kind::INT );
    if ( !numElementsToken )
    {
        throw std::runtime_error( "Expected numeric value" );
    }
    tokens.push_back( numElementsToken.value() );

    auto parseInt = []( const Token& token, std::istream& stream ) {
        int length = Token::binaryTokenSizeInBytes( Token::Kind::INT );

        auto start = token.start();

        int myint;
        stream.seekg( start );
        stream.read( reinterpret_cast<char*>( &myint ), length );

        return myint;
    };

    auto streamPos   = stream.tellg();
    int  numElements = parseInt( numElementsToken.value(), stream );
    stream.seekg( streamPos );

    std::vector<Token> arrayTokens = tokenizeArrayData( stream, numElements, typeToken.kind() );
    tokens.insert( tokens.end(), arrayTokens.begin(), arrayTokens.end() );

    return tokens;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Token> BinaryTokenizer::tokenizeArrayData( std::istream& stream, size_t numElements, Token::Kind kind )
{
    if ( kind == Token::Kind::CHAR )
    {
        assert( false && "Not implemented" );
        std::vector<Token> tokens;
        return tokens;
    }
    else
    {
        auto start = stream.tellg();

        auto length = static_cast<size_t>( Token::binaryTokenSizeInBytes( kind ) ) * numElements;
        auto end    = static_cast<size_t>( start ) + length;

        stream.seekg( end );
        return { Token( Token::Kind::ARRAYBLOB, start, end ) };
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Token> BinaryTokenizer::tokenizeStream( std::istream& stream )
{
    return Tokenizer::tokenizeStream( stream, Token::Kind::ROFF_BIN );
}
