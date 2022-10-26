#include "Tokenizer.hpp"

Token Tokenizer::tokenizeComment( std::istream& stream )
{
    return Token( Token::Kind::TAG, 0, 0 );
}
