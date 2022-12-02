#pragma once

#include "Token.hpp"

#include <istream>
#include <optional>
#include <vector>

class Tokenizer
{
public:
    virtual ~Tokenizer();

    virtual std::vector<Token>   tokenizeStream( std::istream& stream )                                             = 0;
    virtual bool                 tokenizeSpace( std::istream& stream )                                              = 0;
    virtual std::optional<Token> tokenizeString( std::istream& stream )                                             = 0;
    virtual Token                tokenizeName( std::istream& stream )                                               = 0;
    virtual std::vector<Token>   tokenizeArrayTagKey( std::istream& stream )                                        = 0;
    virtual std::optional<Token> tokenizeWord( std::istream& stream, const std::string& keywork, Token::Kind kind ) = 0;

    virtual std::vector<Token> tokenizeTagKey( std::istream& stream );
    std::vector<Token>         tokenizeStream( std::istream& stream, Token::Kind expectedKind );
    virtual std::vector<Token> tokenizeTagGroup( std::istream& stream );
    virtual bool               tokenizeComment( std::istream& stream );
    virtual void               tokenizeDelimiter( std::istream& stream );
    virtual Token              tokenizeSimpleType( std::istream& stream );
    virtual Token              tokenizeKeyword( std::istream& stream );
    virtual Token tokenizeKeyword( std::istream& stream, const std::vector<std::pair<Token::Kind, std::string>>& keywords );

protected:
    virtual std::vector<Token> tokenizeTagKeyInternal( std::istream& stream ) = 0;
};
