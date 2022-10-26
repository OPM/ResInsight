#pragma once

#include "Token.hpp"

#include <istream>

class Tokenizer
{
public:
    static Token tokenizeComment( std::istream& input );
};
