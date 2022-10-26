#include "Tokenizer.hpp"

#include <cctype>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Token Tokenizer::tokenizeComment( std::istream& stream )
{
    auto start    = stream.tellg();
    char readChar = stream.get();
    if ( readChar != '#' )
    {
        stream.seekg( start );
        throw std::runtime_error( "Expected comment." );
    }

    auto end = stream.tellg();
    readChar = stream.get();
    while ( stream.good() && readChar != '#' )
    {
        readChar = stream.get();
    }

    if ( !stream.good() )
    {
        stream.seekg( start );
        throw std::runtime_error( "Reached end of stream while reading comment." );
    }

    end = stream.tellg();
    return Token( Token::Kind::COMMENT, start, end );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Tokenizer::tokenizeSpace( std::istream& stream )
{
    auto start    = stream.tellg();
    char readChar = stream.get();
    if ( !std::isspace( readChar ) )
    {
        stream.seekg( start );
        throw std::runtime_error( "Expected space." );
    }

    auto end = stream.tellg();
    readChar = stream.get();
    while ( stream.good() && std::isspace( readChar ) )
    {
        end      = stream.tellg();
        readChar = stream.get();
    }

    stream.seekg( end );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Token Tokenizer::tokenizeString( std::istream& stream )
{
    tokenizeDelimiter( stream );

    // First part of string should be double-quote
    auto start    = stream.tellg();
    char readChar = stream.get();
    if ( readChar != '"' )
    {
        stream.seekg( start );
        throw std::runtime_error( "Expected string." );
    }

    // Read until closing double-quote.
    start    = stream.tellg();
    auto end = stream.tellg();
    readChar = stream.get();
    while ( stream.good() && readChar != '"' )
    {
        end      = stream.tellg();
        readChar = stream.get();
    }

    if ( !stream.good() )
    {
        // Reached unexpected end of file.
        stream.seekg( start );
        throw std::runtime_error( "Reached end of stream while reading string literal." );
    }

    return Token( Token::Kind::STRING_LITERAL, start, end );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Tokenizer::tokenizeDelimiter( std::istream& stream )
{
    // TODO: improve this code
    auto f1 = []( std::istream& s ) {
        try
        {
            tokenizeSpace( s );
            return true;
        }
        catch ( const std::runtime_error& e )
        {
            return false;
        }
    };

    auto f2 = []( std::istream& s ) {
        try
        {
            tokenizeComment( s );
            return true;
        }
        catch ( const std::runtime_error& e )
        {
            return false;
        }
    };

    while ( stream.good() && ( f1( stream ) || f2( stream ) ) )
    {
    }
}
