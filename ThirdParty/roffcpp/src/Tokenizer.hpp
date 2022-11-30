#pragma once

#include "Token.hpp"

#include <istream>
#include <optional>
#include <vector>

class Tokenizer
{
public:
    virtual ~Tokenizer();

    virtual std::vector<Token>   tokenizeStream( std::istream& stream )           = 0;
    virtual bool                 tokenizeComment( std::istream& stream )          = 0;
    virtual bool                 tokenizeSpace( std::istream& stream )            = 0;
    virtual void                 tokenizeDelimiter( std::istream& stream )        = 0;
    virtual std::optional<Token> tokenizeString( std::istream& stream )           = 0;
    virtual std::optional<Token> tokenizeAsciiNumber( std::istream& stream )      = 0;
    virtual Token                tokenizeName( std::istream& stream )             = 0;
    virtual Token                tokenizeKeyword( std::istream& stream )          = 0;
    virtual Token                tokenizeSimpleType( std::istream& stream )       = 0;
    virtual std::optional<Token> tokenizeValue( std::istream& stream )            = 0;
    virtual std::vector<Token>   tokenizeArrayData( std::istream& stream )        = 0;
    virtual std::vector<Token>   tokenizeAsciiTagKey( std::istream& stream )      = 0;
    virtual std::vector<Token>   tokenizeArrayAsciiTagKey( std::istream& stream ) = 0;
    virtual std::vector<Token>   tokenizeTagGroup( std::istream& stream )         = 0;
    virtual std::vector<Token>   tokenizeTagKey( std::istream& stream )           = 0;

    virtual Token                tokenizeKeyword( std::istream&                                           stream,
                                                  const std::vector<std::pair<Token::Kind, std::string>>& keywords ) = 0;
    virtual std::optional<Token> tokenizeWord( std::istream& stream, const std::string& keywork, Token::Kind kind ) = 0;
};
