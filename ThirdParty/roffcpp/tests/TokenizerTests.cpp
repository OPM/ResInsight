
#include "gtest/gtest.h"

#include <string>

#include "Tokenizer.hpp"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( TokenizerTests, testTokenizeComment )
{
    Token token = Tokenizer::tokenizeComment( "# This is a comment. #" );
    ASSERT_EQ( Token::Kind::COMMENT, token.kind() );
}
