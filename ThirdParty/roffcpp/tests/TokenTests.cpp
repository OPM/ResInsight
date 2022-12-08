
#include "gtest/gtest.h"

#include <string>

#include "Token.hpp"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( TokenTests, SimpleTest )
{
    size_t start = 3u;
    size_t end   = 6u;
    Token  token( Token::Kind::TAG, start, end );

    ASSERT_EQ( Token::Kind::TAG, token.kind() );
    ASSERT_EQ( start, token.start() );
    ASSERT_EQ( end, token.end() );
}
