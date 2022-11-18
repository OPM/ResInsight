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
    parse();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Parser::parse()
{
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
            m_scalarValues.push_back( parseSimpleType( it, tagGroupName ) );
        }
        else if ( it->kind() == Token::Kind::ARRAY )
        {
            // Extract the type
            std::advance( it, 1 );
            Token typeToken = *it;
            assert( isSimpleType( typeToken.kind() ) );

            // Extract the name
            std::advance( it, 1 );
            std::string name = tagGroupName + "." + parseString( *it, *m_stream );

            // Extract number of array items
            std::advance( it, 1 );
            long arrayLength = parseInt( *it, *m_stream );

            // Move to the first item to get the index
            std::advance( it, 1 );
            long startIndex = static_cast<long>(std::distance( m_tokens->begin(), it ));

            // Skip rest of the array
            std::advance( it, arrayLength - 1 );

            m_arrayInfo[name] = std::make_pair( startIndex, arrayLength );

            m_arrayTypes.push_back( std::make_pair( name, typeToken.kind() ) );
        }
        else if ( it->kind() == Token::Kind::ENDTAG )
        {
            tagGroupName = "";
        }
        it++;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<std::string, RoffScalar>> Parser::scalarNamedValues() const
{
    return m_scalarValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<std::string, Token::Kind>> Parser::getNamedArrayTypes() const
{
    return m_arrayTypes;
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
    size_t         length = token.end() - token.start();
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> Parser::getStringArray( const std::string& keyword )
{
    auto [startIndex, arrayLength] = m_arrayInfo[keyword];

    std::vector<std::string> values;
    for ( long i = startIndex; i < startIndex + arrayLength; i++ )
    {
        values.push_back( parseString( ( *m_tokens )[i], *m_stream ) );
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> Parser::getIntArray( const std::string& keyword )
{
    auto [startIndex, arrayLength] = m_arrayInfo[keyword];

    std::vector<int> values;
    for ( long i = startIndex; i < startIndex + arrayLength; i++ )
    {
        values.push_back( parseInt( ( *m_tokens )[i], *m_stream ) );
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<char> Parser::getByteArray( const std::string& keyword )
{
    auto [startIndex, arrayLength] = m_arrayInfo[keyword];

    std::vector<char> values;
    for ( long i = startIndex; i < startIndex + arrayLength; i++ )
    {
        values.push_back( static_cast<char>(parseInt( ( *m_tokens )[i], *m_stream ) ));
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<float> Parser::getFloatArray( const std::string& keyword )
{
    auto [startIndex, arrayLength] = m_arrayInfo[keyword];

    std::vector<float> values;
    for ( long i = startIndex; i < startIndex + arrayLength; i++ )
    {
        values.push_back( static_cast<float>(parseDouble( ( *m_tokens )[i], *m_stream ) ));
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> Parser::getDoubleArray( const std::string& keyword )
{
    auto [startIndex, arrayLength] = m_arrayInfo[keyword];

    std::vector<double> values;
    for ( long i = startIndex; i < startIndex + arrayLength; i++ )
    {
        values.push_back( parseDouble( ( *m_tokens )[i], *m_stream ) );
    }

    return values;
}
