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
};
