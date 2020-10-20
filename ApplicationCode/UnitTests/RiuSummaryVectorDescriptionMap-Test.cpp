#include "gtest/gtest.h"

#include "RiuSummaryQuantityNameInfoProvider.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiuSummaryQuantityNameInfoProvider, TestInit )
{
    {
        std::string s( "SRSFC" );
        auto        cat = RiuSummaryQuantityNameInfoProvider::instance()->categoryFromQuantityName( s );
        EXPECT_TRUE( cat == RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT );

        auto longName = RiuSummaryQuantityNameInfoProvider::instance()->longNameFromQuantityName( s );
        EXPECT_TRUE( longName == "Reach brine concentration" );
    }

    {
        std::string s( "SRSFC" );
        auto        test = RiuSummaryQuantityNameInfoProvider::instance()->longNameFromQuantityName( s );

        EXPECT_TRUE( test == "Reach brine concentration" );
    }

    {
        std::string s( "does not exist" );
        auto        cat = RiuSummaryQuantityNameInfoProvider::instance()->categoryFromQuantityName( s );
        EXPECT_TRUE( cat == RifEclipseSummaryAddress::SUMMARY_INVALID );

        auto longName = RiuSummaryQuantityNameInfoProvider::instance()->longNameFromQuantityName( s );

        EXPECT_TRUE( longName == "" );
    }

    {
        std::string s( "does not exist" );
        auto        test = RiuSummaryQuantityNameInfoProvider::instance()->longNameFromQuantityName( s );

        EXPECT_TRUE( test == "" );
    }

    {
        std::string s( "does not exist" );
        auto        test = RiuSummaryQuantityNameInfoProvider::instance()->longNameFromQuantityName( s, true );

        EXPECT_TRUE( test == s );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiuSummaryQuantityNameInfoProvider, TestCustomNaming )
{
    {
        std::string s( "SRSFCABC" );
        auto        cat = RiuSummaryQuantityNameInfoProvider::instance()->categoryFromQuantityName( s );
        EXPECT_TRUE( cat == RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT );

        auto longName = RiuSummaryQuantityNameInfoProvider::instance()->longNameFromQuantityName( s );
        EXPECT_TRUE( longName == "Reach brine concentration" );
    }

    {
        std::string s( "BHD__ABC" );
        auto        test = RiuSummaryQuantityNameInfoProvider::instance()->longNameFromQuantityName( s );

        EXPECT_TRUE( test == "Hydraulic head" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiuSummaryQuantityNameInfoProvider, Test6x )
{
    {
        std::string s( "GLIT" );
        auto        cat = RiuSummaryQuantityNameInfoProvider::instance()->categoryFromQuantityName( s );
        EXPECT_TRUE( cat == RifEclipseSummaryAddress::SUMMARY_WELL_GROUP );
    }

    {
        std::string s( "BDYNKZp" );
        auto        cat = RiuSummaryQuantityNameInfoProvider::instance()->categoryFromQuantityName( s );
        EXPECT_TRUE( cat == RifEclipseSummaryAddress::SUMMARY_BLOCK );
    }
}
