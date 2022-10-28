#pragma once

#include "Token.hpp"

#include <istream>

class Tokenizer
{
public:
    static Token tokenizeComment( std::istream& stream );
    static void  tokenizeSpace( std::istream& stream );
    static void  tokenizeDelimiter( std::istream& stream );
    static Token tokenizeString( std::istream& stream );
    static Token tokenizeAsciiNumber( std::istream& stream );
    static Token tokenizeName( std::istream& stream );
    static Token tokenizeKeyword( std::istream& stream );
    static Token tokenizeWord( std::istream& stream, const std::string& keywork, Token::Kind kind );
};
