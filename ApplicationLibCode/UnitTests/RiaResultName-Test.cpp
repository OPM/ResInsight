#include "gtest/gtest.h"

#include "RiaResultNames.h"

TEST( RiaResultNames, TestIsCategoryResult )
{
    // Test for non-category result names
    {
        QString resultVariable = "FIPOIL";
        bool    result         = RiaResultNames::isCategoryResult( resultVariable );
        EXPECT_FALSE( result );
    }

    {
        QString resultVariable = "FIPWAT";
        bool    result         = RiaResultNames::isCategoryResult( resultVariable );
        EXPECT_FALSE( result );
    }

    {
        QString resultVariable = "FIPGAS";
        bool    result         = RiaResultNames::isCategoryResult( resultVariable );
        EXPECT_FALSE( result );
    }

    {
        QString resultVariable = "SFIPGAS";
        bool    result         = RiaResultNames::isCategoryResult( resultVariable );
        EXPECT_FALSE( result );
    }

    {
        QString resultVariable = "RFIPGAS";
        bool    result         = RiaResultNames::isCategoryResult( resultVariable );
        EXPECT_FALSE( result );
    }

    {
        QString resultVariable = "RFIPGAS_P90";
        bool    result         = RiaResultNames::isCategoryResult( resultVariable );
        EXPECT_FALSE( result );
    }

    // Test for category result names
    {
        QString resultVariable = "MYNUM";
        bool    result         = RiaResultNames::isCategoryResult( resultVariable );
        EXPECT_TRUE( result );
    }

    {
        QString resultVariable = "FIPMYPROP";
        bool    result         = RiaResultNames::isCategoryResult( resultVariable );
        EXPECT_TRUE( result );
    }
}
