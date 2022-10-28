#pragma once

#include "Token.hpp"

#include <istream>
#include <vector>

class Tokenizer
{
public:
    static Token              tokenizeComment( std::istream& stream );
    static void               tokenizeSpace( std::istream& stream );
    static void               tokenizeDelimiter( std::istream& stream );
    static Token              tokenizeString( std::istream& stream );
    static Token              tokenizeAsciiNumber( std::istream& stream );
    static Token              tokenizeName( std::istream& stream );
    static Token              tokenizeKeyword( std::istream& stream );
    static Token              tokenizeSimpleType( std::istream& stream );
    static Token              tokenizeValue( std::istream& stream );
    static std::vector<Token> tokenizeAsciiTagKey( std::istream& stream );

    static Token tokenizeKeyword( std::istream& stream, const std::vector<std::pair<Token::Kind, std::string>>& keywords );
    static Token tokenizeWord( std::istream& stream, const std::string& keywork, Token::Kind kind );
};
