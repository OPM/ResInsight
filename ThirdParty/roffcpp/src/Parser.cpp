#include "Parser.hpp"

#include "RoffScalar.hpp"

#include <cassert>
#include <cctype>
#include <stdexcept>
#include <variant>
#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Parser::Parser()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Parser::~Parser()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Parser::parse( std::istream&                                     stream,
                    const std::vector<Token>&                         tokens,
                    std::vector<std::pair<std::string, RoffScalar>>&  scalarValues,
                    std::vector<std::pair<std::string, Token::Kind>>& arrayTypes,
                    std::map<std::string, std::pair<long, long>>&     arrayInfo ) const
{
    auto        it           = tokens.begin();
    std::string tagGroupName = "";
    while ( it != tokens.end() )
    {
        if ( it->kind() == Token::Kind::TAG )
        {
            std::advance( it, 1 );
            assert( it->kind() == Token::Kind::NAME );
            tagGroupName = parseString( *it, stream );
        }
        else if ( isSimpleType( it->kind() ) )
        {
            scalarValues.push_back( parseSimpleType( it, tagGroupName, stream ) );
        }
        else if ( it->kind() == Token::Kind::ARRAY )
        {
            // Extract the type
            std::advance( it, 1 );
            Token typeToken = *it;
            assert( isSimpleType( typeToken.kind() ) );

            // Extract the name
            std::advance( it, 1 );
            std::string name = tagGroupName + "." + parseString( *it, stream );

            // Extract number of array items
            std::advance( it, 1 );
            long arrayLength = parseInt( *it, stream );

            // Move to the first item to get the index
            std::advance( it, 1 );
            long startIndex = static_cast<long>( std::distance( tokens.begin(), it ) );

            if ( it->kind() != Token::Kind::ARRAYBLOB )
            {
                // Skip rest of the array when not blob.
                // The blob is always only one token and is skip already.
                std::advance( it, arrayLength - 1 );
            }

            arrayInfo[name] = std::make_pair( startIndex, arrayLength );

            arrayTypes.push_back( std::make_pair( name, typeToken.kind() ) );
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
bool Parser::isSimpleType( Token::Kind kind )
{
    return kind == Token::Kind::INT || kind == Token::Kind::BOOL || kind == Token::Kind::BYTE ||
           kind == Token::Kind::FLOAT || kind == Token::Kind::DOUBLE || kind == Token::Kind::CHAR;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::string, RoffScalar> Parser::parseSimpleType( std::vector<Token>::const_iterator& it,
                                                            const std::string&                  tagGroupName,
                                                            std::istream&                       stream ) const
{
    assert( isSimpleType( it->kind() ) );
    Token::Kind kind = it->kind();

    std::advance( it, 1 );
    assert( it->kind() == Token::Kind::NAME );
    std::string name = parseString( *it, stream );

    std::advance( it, 1 );

    auto extractValue = [this]( auto it, Token::Kind kind, std::istream& stream ) {
        if ( kind == Token::Kind::INT ) return RoffScalar( parseInt( *it, stream ) );
        if ( kind == Token::Kind::BOOL ) return RoffScalar( parseBool( *it, stream ) );
        if ( kind == Token::Kind::BYTE ) return RoffScalar( parseByte( *it, stream ) );
        if ( kind == Token::Kind::CHAR ) return RoffScalar( parseString( *it, stream ) );
        if ( kind == Token::Kind::DOUBLE ) return RoffScalar( parseDouble( *it, stream ) );
        if ( kind == Token::Kind::FLOAT ) return RoffScalar( parseFloat( *it, stream ) );
        return RoffScalar( 1 );
    };

    RoffScalar val = extractValue( it, kind, stream );

    return std::make_pair( tagGroupName + "." + name, val );
}
