#include "Tokenizer.hpp"

Tokenizer::~Tokenizer()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Tokenizer::tokenizeComment( std::istream& stream )
{
    auto          start    = stream.tellg();
    unsigned char readChar = static_cast<unsigned char>( stream.get() );
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

    tokenizeDelimiter( stream );
    return tokenizeKeyword( stream, keywords );
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
Token Tokenizer::tokenizeKeyword( std::istream& stream, const std::vector<std::pair<Token::Kind, std::string>>& keywords )
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
void Tokenizer::tokenizeDelimiter( std::istream& stream )
{
    while ( stream.good() && ( tokenizeSpace( stream ) || tokenizeComment( stream ) ) )
    {
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Token> Tokenizer::tokenizeTagGroup( std::istream& stream )
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
std::vector<Token> Tokenizer::tokenizeStream( std::istream& stream, Token::Kind expectedKind )
{
    std::vector<Token> tokens;

    tokens.push_back( tokenizeKeyword( stream, { std::make_pair( expectedKind, Token::kindToString( expectedKind ) ) } ) );

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
std::vector<Token> Tokenizer::tokenizeTagKey( std::istream& stream )
{
    try
    {
        return tokenizeTagKeyInternal( stream );
    }
    catch ( const std::runtime_error& )
    {
        return tokenizeArrayTagKey( stream );
    }
}
