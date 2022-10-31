#include "Token.hpp"

#include <cassert>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Token::Token( Token::Kind kind, size_t start, size_t end )
    : m_kind( kind )
    , m_start( start )
    , m_end( end )
{
    assert( start <= end );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Token::Kind Token::kind() const
{
    return m_kind;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t Token::start() const
{
    return m_start;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t Token::end() const
{
    return m_end;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string Token::kindToString( Token::Kind kind )
{
    switch ( kind )
    {
        case Token::Kind::ROFF_BIN:
            return "roff-bin";
        case Token::Kind::ROFF_ASC:
            return "roff-asc";
        case Token::Kind::TAG:
            return "tag";
        case Token::Kind::ENDTAG:
            return "endtag";
        case Token::Kind::CHAR:
            return "char";
        case Token::Kind::BOOL:
            return "bool";
        case Token::Kind::BYTE:
            return "byte";
        case Token::Kind::INT:
            return "int";
        case Token::Kind::FLOAT:
            return "float";
        case Token::Kind::DOUBLE:
            return "double";
        case Token::Kind::ARRAY:
            return "array";
        case Token::Kind::COMMENT:
            return "comment";
        case Token::Kind::STRING_LITERAL:
            return "string";
        case Token::Kind::NAME:
            return "name";
        case Token::Kind::BINARY_NUMERIC_VALUE:
            return "binary number";
        case Token::Kind::ARRAYBLOB:
            return "array-blob";
        case Token::Kind::NUMERIC_VALUE:
            return "number";
        default:
            assert( false );
    }

    return "";
}
