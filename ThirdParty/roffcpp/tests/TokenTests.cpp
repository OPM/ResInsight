
#include "gtest/gtest.h"

#include <string>

#include "Token.hpp"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( TokenTests, SimpleTest )
{
    Token token( Token::Kind::TAG, 3, 6 );

    ASSERT_EQ( Token::Kind::TAG, token.kind() );
    ASSERT_EQ( 3, token.start() );
    ASSERT_EQ( 6, token.end() );
}
