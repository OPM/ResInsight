#pragma once

#include "Token.hpp"

#include <istream>
#include <optional>
#include <vector>

class Tokenizer
{
public:
    static std::vector<Token>   tokenizeStream( std::istream& stream );
    static bool                 tokenizeComment( std::istream& stream );
    static bool                 tokenizeSpace( std::istream& stream );
    static void                 tokenizeDelimiter( std::istream& stream );
    static std::optional<Token> tokenizeString( std::istream& stream );
    static std::optional<Token> tokenizeAsciiNumber( std::istream& stream );
    static Token                tokenizeName( std::istream& stream );
    static Token                tokenizeKeyword( std::istream& stream );
    static Token                tokenizeSimpleType( std::istream& stream );
    static std::optional<Token> tokenizeValue( std::istream& stream );
    static std::vector<Token>   tokenizeArrayData( std::istream& stream );
    static std::vector<Token>   tokenizeAsciiTagKey( std::istream& stream );
    static std::vector<Token>   tokenizeArrayAsciiTagKey( std::istream& stream );
    static std::vector<Token>   tokenizeTagGroup( std::istream& stream );
    static std::vector<Token>   tokenizeTagKey( std::istream& stream );

    static Token tokenizeKeyword( std::istream& stream, const std::vector<std::pair<Token::Kind, std::string>>& keywords );
    static std::optional<Token> tokenizeWord( std::istream& stream, const std::string& keywork, Token::Kind kind );
};
