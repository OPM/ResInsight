#include "AsciiTokenizer.hpp"

#include <cctype>
#include <optional>
#include <stdexcept>
#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool AsciiTokenizer::tokenizeComment( std::istream& stream )
{
    auto start    = stream.tellg();
    char readChar = static_cast<char>( stream.get() );
    if ( readChar != '#' )
    {
        stream.seekg( start );
        return false;
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
        return false;
    }

    end = stream.tellg();
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool AsciiTokenizer::tokenizeSpace( std::istream& stream )
{
    auto start    = stream.tellg();
    char readChar = static_cast<char>( stream.get() );
    if ( !std::isspace( readChar ) )
    {
        stream.seekg( start );
        return false;
    }

    auto end = stream.tellg();
    readChar = static_cast<char>( stream.get() );
    while ( stream.good() && std::isspace( readChar ) )
    {
        end      = stream.tellg();
        readChar = static_cast<char>( stream.get() );
    }

    stream.seekg( end );
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<Token> AsciiTokenizer::tokenizeString( std::istream& stream )
{
    tokenizeDelimiter( stream );

    // First part of string should be double-quote
    auto start    = stream.tellg();
    char readChar = static_cast<char>( stream.get() );
    if ( readChar != '"' )
    {
        // Expected string, but no opening quote found
        stream.seekg( start );
        return {};
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
        return {};
    }

    return Token( Token::Kind::STRING_LITERAL, start, end );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Token AsciiTokenizer::tokenizeName( std::istream& stream )
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
std::optional<Token> AsciiTokenizer::tokenizeAsciiNumber( std::istream& stream )
{
    tokenizeDelimiter( stream );

    auto isCharValidInDigit = []( char c ) {
        return std::isdigit( c ) || c == '.' || c == 'e' || c == 'E' || c == '-' || c == '+';
    };

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
        // Expected numeric value, but could not find one.
        stream.seekg( start );
        return {};
    }

    stream.seekg( end );
    return Token( Token::Kind::NUMERIC_VALUE, start, end );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Token AsciiTokenizer::tokenizeKeyword( std::istream& stream )
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

    tokenizeDelimiter( stream );
    return tokenizeKeyword( stream, keywords );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Token AsciiTokenizer::tokenizeKeyword( std::istream&                                           stream,
                                       const std::vector<std::pair<Token::Kind, std::string>>& keywords )
{
    for ( auto [kind, keyword] : keywords )
    {
        auto token = tokenizeWord( stream, keyword, kind );
        if ( token )
        {
            return token.value();
        }
    }

    throw std::runtime_error( "No matching keyword." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<Token> AsciiTokenizer::tokenizeValue( std::istream& stream )
{
    auto numberToken = tokenizeAsciiNumber( stream );
    if ( numberToken )
        return numberToken;
    else
        return tokenizeString( stream );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Token AsciiTokenizer::tokenizeSimpleType( std::istream& stream )
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
std::optional<Token> AsciiTokenizer::tokenizeWord( std::istream& stream, const std::string& keyword, Token::Kind kind )
{
    tokenizeDelimiter( stream );
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
        return {};
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Token> AsciiTokenizer::tokenizeAsciiTagKey( std::istream& stream )
{
    std::vector<Token> tokens;
    tokens.push_back( tokenizeSimpleType( stream ) );
    tokens.push_back( tokenizeName( stream ) );
    auto value = tokenizeValue( stream );
    if ( value )
        tokens.push_back( value.value() );
    else
        throw std::runtime_error( "Invalid value." );
    return tokens;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Token> AsciiTokenizer::tokenizeArrayAsciiTagKey( std::istream& stream )
{
    std::vector<Token> tokens;
    tokens.push_back( tokenizeKeyword( stream, { std::make_pair( Token::Kind::ARRAY, "array" ) } ) );
    tokens.push_back( tokenizeSimpleType( stream ) );
    tokens.push_back( tokenizeName( stream ) );
    auto numberOfElements = tokenizeAsciiNumber( stream );
    if ( !numberOfElements ) throw std::runtime_error( "Expected numeric value" );

    tokens.push_back( numberOfElements.value() );

    std::vector<Token> arrayTokens = tokenizeArrayData( stream );
    tokens.insert( tokens.end(), arrayTokens.begin(), arrayTokens.end() );

    return tokens;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Token> AsciiTokenizer::tokenizeArrayData( std::istream& stream )
{
    std::vector<Token> tokens;

    bool gotNewToken = true;
    while ( gotNewToken )
    {
        auto token = tokenizeValue( stream );
        if ( token )
            tokens.push_back( token.value() );
        else
            gotNewToken = false;
    }

    return tokens;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Token> AsciiTokenizer::tokenizeTagGroup( std::istream& stream )
{
    tokenizeDelimiter( stream );
    std::vector<Token> tokens;
    tokens.push_back( tokenizeKeyword( stream, { std::make_pair( Token::Kind::TAG, "tag" ) } ) );

    tokens.push_back( tokenizeName( stream ) );

    bool hasMoreTokens = true;
    while ( hasMoreTokens )
    {
        try
        {
            std::vector<Token> tagGroupTokens = tokenizeTagKey( stream );

            for ( const Token& tok : tagGroupTokens )
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
std::vector<Token> AsciiTokenizer::tokenizeTagKey( std::istream& stream )
{
    try
    {
        return tokenizeAsciiTagKey( stream );
    }
    catch ( const std::runtime_error& )
    {
        return tokenizeArrayAsciiTagKey( stream );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Token> AsciiTokenizer::tokenizeStream( std::istream& stream )
{
    std::vector<Token> tokens;
    tokens.push_back( tokenizeKeyword( stream, { std::make_pair( Token::Kind::ROFF_ASC, "roff-asc" ) } ) );

    bool hasMoreTokens = true;
    while ( hasMoreTokens )
    {
        try
        {
            std::vector<Token> tagGroupTokens = tokenizeTagGroup( stream );
            for ( const Token& tok : tagGroupTokens )
                tokens.push_back( tok );

            hasMoreTokens = true;
        }
        catch ( std::runtime_error& )
        {
            hasMoreTokens = false;
        }
    }

    return tokens;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void AsciiTokenizer::tokenizeDelimiter( std::istream& stream )
{
    while ( stream.good() && ( tokenizeSpace( stream ) || tokenizeComment( stream ) ) )
    {
    }
}
