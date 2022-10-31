#include "Tokenizer.hpp"

#include <cctype>
#include <stdexcept>
#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Token Tokenizer::tokenizeComment( std::istream& stream )
{
    auto start    = stream.tellg();
    char readChar = static_cast<char>( stream.get() );
    if ( readChar != '#' )
    {
        stream.seekg( start );
        throw std::runtime_error( "Expected comment." );
    }

    auto end = stream.tellg();
    readChar = static_cast<char>( stream.get() );
    while ( stream.good() && readChar != '#' )
    {
        readChar = static_cast<char>( stream.get() );
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
    char readChar = static_cast<char>( stream.get() );
    if ( !std::isspace( readChar ) )
    {
        stream.seekg( start );
        throw std::runtime_error( "Expected space." );
    }

    auto end = stream.tellg();
    readChar = static_cast<char>( stream.get() );
    while ( stream.good() && std::isspace( readChar ) )
    {
        end      = stream.tellg();
        readChar = static_cast<char>( stream.get() );
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
    char readChar = static_cast<char>( stream.get() );
    if ( readChar != '"' )
    {
        stream.seekg( start );
        throw std::runtime_error( "Expected string." );
    }

    // Read until closing double-quote.
    start    = stream.tellg();
    auto end = start;
    readChar = static_cast<char>( stream.get() );
    while ( stream.good() && readChar != '"' )
    {
        end      = stream.tellg();
        readChar = static_cast<char>( stream.get() );
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
Token Tokenizer::tokenizeName( std::istream& stream )
{
    tokenizeDelimiter( stream );

    auto start  = stream.tellg();
    int  length = 0;

    auto readChar = static_cast<char>( stream.get() );
    while ( stream.good() && !std::isspace( readChar ) )
    {
        length++;
        readChar = static_cast<char>( stream.get() );
    }

    if ( length < 1 )
    {
        stream.seekg( start );
        throw std::runtime_error( "Could not tokenize name." );
    }

    return Token( Token::Kind::NAME, start, static_cast<size_t>( start ) + length );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Token Tokenizer::tokenizeAsciiNumber( std::istream& stream )
{
    tokenizeDelimiter( stream );

    auto isCharValidInDigit = []( char c ) { return std::isdigit( c ) || c == '.' || c == 'e' || c == 'E' || c == '-'; };

    auto start = stream.tellg();
    auto end   = start;

    char readChar = static_cast<char>( stream.get() );
    if ( stream.good() && ( std::isdigit( readChar ) || readChar == '-' ) )
    {
        while ( stream.good() && isCharValidInDigit( readChar ) )
        {
            end      = stream.tellg();
            readChar = static_cast<char>( stream.get() );
        }
    }

    if ( end - start < 1 )
    {
        stream.seekg( start );
        throw std::runtime_error( "Expected numeric value" );
    }

    stream.seekg( end );
    return Token( Token::Kind::NUMERIC_VALUE, start, end );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Token Tokenizer::tokenizeKeyword( std::istream& stream )
{
    std::vector<std::pair<Token::Kind, std::string>> keywords = {
        std::make_pair( Token::Kind::ROFF_BIN, "roff-bin" ),
        std::make_pair( Token::Kind::ROFF_ASC, "roff-asc" ),
        std::make_pair( Token::Kind::TAG, "tag" ),
        std::make_pair( Token::Kind::ENDTAG, "endtag" ),
        std::make_pair( Token::Kind::CHAR, "char" ),
        std::make_pair( Token::Kind::BOOL, "bool" ),
        std::make_pair( Token::Kind::BYTE, "byte" ),
        std::make_pair( Token::Kind::INT, "int" ),
        std::make_pair( Token::Kind::FLOAT, "float" ),
        std::make_pair( Token::Kind::DOUBLE, "double" ),
        std::make_pair( Token::Kind::ARRAY, "array" ),
    };

    return tokenizeKeyword( stream, keywords );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Token Tokenizer::tokenizeKeyword( std::istream& stream, const std::vector<std::pair<Token::Kind, std::string>>& keywords )
{
    for ( auto [kind, keyword] : keywords )
    {
        try
        {
            return tokenizeWord( stream, keyword, kind );
        }
        catch ( std::runtime_error& )
        {
        }
    }

    throw std::runtime_error( "No matching keyword." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Token Tokenizer::tokenizeValue( std::istream& stream )
{
    try
    {
        return tokenizeAsciiNumber( stream );
    }
    catch ( const std::runtime_error& )
    {
        return tokenizeString( stream );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Token Tokenizer::tokenizeSimpleType( std::istream& stream )
{
    std::vector<std::pair<Token::Kind, std::string>> simpleTypes = { std::make_pair( Token::Kind::CHAR, "char" ),
                                                                     std::make_pair( Token::Kind::BOOL, "bool" ),
                                                                     std::make_pair( Token::Kind::BYTE, "byte" ),
                                                                     std::make_pair( Token::Kind::INT, "int" ),
                                                                     std::make_pair( Token::Kind::FLOAT, "float" ),
                                                                     std::make_pair( Token::Kind::DOUBLE, "double" ) };
    return tokenizeKeyword( stream, simpleTypes );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Token Tokenizer::tokenizeWord( std::istream& stream, const std::string& keyword, Token::Kind kind )
{
    auto        start = stream.tellg();
    std::string word;
    if ( stream >> word && word == keyword )
    {
        auto end = static_cast<size_t>( start ) + keyword.length();
        return Token( kind, start, end );
    }
    else
    {
        stream.seekg( start );
        throw std::runtime_error( "Token did not match word: " + keyword );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Token> Tokenizer::tokenizeAsciiTagKey( std::istream& stream )
{
    std::vector<Token> tokens;
    tokens.push_back( tokenizeSimpleType( stream ) );
    tokens.push_back( tokenizeName( stream ) );
    tokens.push_back( tokenizeValue( stream ) );
    return tokens;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Token> Tokenizer::tokenizeArrayAsciiTagKey( std::istream& stream )
{
    std::vector<Token> tokens;
    tokens.push_back( tokenizeKeyword( stream, { std::make_pair( Token::Kind::ARRAY, "array" ) } ) );
    tokens.push_back( tokenizeSimpleType( stream ) );
    tokens.push_back( tokenizeName( stream ) );
    tokens.push_back( tokenizeAsciiNumber( stream ) );

    std::vector<Token> arrayTokens = tokenizeArrayData( stream );
    tokens.insert( tokens.end(), arrayTokens.begin(), arrayTokens.end() );

    return tokens;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Token> Tokenizer::tokenizeArrayData( std::istream& stream )
{
    std::vector<Token> tokens;

    bool gotNewToken = true;
    while ( gotNewToken )
    {
        try
        {
            tokens.push_back( tokenizeValue( stream ) );
        }
        catch ( std::runtime_error& e )
        {
            gotNewToken = false;
        }
    }

    return tokens;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Token> Tokenizer::tokenizeTagGroup( std::istream& stream )
{
    std::vector<Token> tokens;
    tokens.push_back( tokenizeKeyword( stream, { std::make_pair( Token::Kind::TAG, "tag" ) } ) );

    tokens.push_back( tokenizeName( stream ) );

    bool hasMoreTokens = true;
    while ( hasMoreTokens )
    {
        try
        {
            std::vector<Token> tagGroupTokens = tokenizeAsciiTagKey( stream );
            for ( auto tok : tagGroupTokens )
                tokens.push_back( tok );

            hasMoreTokens = true;
        }
        catch ( std::runtime_error& )
        {
            hasMoreTokens = false;
        }
    }

    tokens.push_back( tokenizeKeyword( stream, { std::make_pair( Token::Kind::ENDTAG, "endtag" ) } ) );

    return tokens;
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
        catch ( const std::runtime_error& )
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
        catch ( const std::runtime_error& )
        {
            return false;
        }
    };

    while ( stream.good() && ( f1( stream ) || f2( stream ) ) )
    {
    }
}
