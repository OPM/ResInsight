#include "Parser.hpp"

#include <cassert>
#include <cctype>
#include <stdexcept>
#include <variant>
#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Parser::Parser( std::istream& stream, const std::vector<Token>& tokens )
    : m_tokens( &tokens )
    , m_stream( &stream )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<std::string, RoffScalar>> Parser::scalarNamedValues()
{
    std::vector<std::pair<std::string, RoffScalar>> values;

    auto        it           = m_tokens->begin();
    std::string tagGroupName = "";
    while ( it != m_tokens->end() )
    {
        if ( it->kind() == Token::Kind::TAG )
        {
            std::advance( it, 1 );
            assert( it->kind() == Token::Kind::NAME );
            tagGroupName = parseString( *it, *m_stream );
        }
        else if ( isSimpleType( it->kind() ) )
        {
            values.push_back( parseSimpleType( it, tagGroupName ) );
        }
        else if ( it->kind() == Token::Kind::ARRAY )
        {
            std::advance( it, 1 );
            // assert simple type?
            std::advance( it, 1 );
        }
        else if ( it->kind() == Token::Kind::ENDTAG )
        {
            tagGroupName = "";
        }
        it++;
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Parser::isSimpleType( Token::Kind kind )
{
    return kind == Token::Kind::INT || kind == Token::Kind::BOOL || kind == Token::Kind::BYTE ||
           kind == Token::Kind::FLOAT || kind == Token::Kind::DOUBLE || kind == Token::Kind::CHAR;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::string, RoffScalar> Parser::parseSimpleType( std::vector<Token>::const_iterator& it,
                                                            const std::string&                  tagGroupName )
{
    assert( isSimpleType( it->kind() ) );
    Token::Kind kind = it->kind();

    std::advance( it, 1 );
    assert( it->kind() == Token::Kind::NAME );
    std::string name = parseString( *it, *m_stream );

    std::advance( it, 1 );

    auto extractValue = []( auto it, Token::Kind kind, std::istream& stream ) {
        if ( kind == Token::Kind::INT ) return RoffScalar( parseInt( *it, stream ) );
        if ( kind == Token::Kind::BOOL ) return RoffScalar( static_cast<bool>( parseInt( *it, stream ) ) );
        if ( kind == Token::Kind::BYTE ) return RoffScalar( static_cast<unsigned char>( parseInt( *it, stream ) ) );
        if ( kind == Token::Kind::CHAR ) return RoffScalar( parseString( *it, stream ) );
        if ( kind == Token::Kind::DOUBLE ) return RoffScalar( parseDouble( *it, stream ) );
        if ( kind == Token::Kind::FLOAT ) return RoffScalar( static_cast<float>( parseDouble( *it, stream ) ) );
        return RoffScalar( 1 );
    };

    RoffScalar val = extractValue( it, kind, *m_stream );

    return std::make_pair( tagGroupName + "." + name, val );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string Parser::parseString( const Token& token, std::istream& stream )
{
    stream.clear();
    stream.seekg( token.start() );
    int         length = token.end() - token.start();
    std::string res;
    res.resize( length );
    stream.read( const_cast<char*>( res.data() ), length );
    return res;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int Parser::parseInt( const Token& token, std::istream& stream )
{
    std::string res = parseString( token, stream );
    return std::stoi( res );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double Parser::parseDouble( const Token& token, std::istream& stream )
{
    std::string res = parseString( token, stream );
    return std::stod( res );
}
