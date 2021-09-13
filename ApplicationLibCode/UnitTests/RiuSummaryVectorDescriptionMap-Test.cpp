#include "gtest/gtest.h"

#include "RiuSummaryQuantityNameInfoProvider.h"
#include <chrono>

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
        std::string s( "WSBVPROP" );
        auto        cat = RiuSummaryQuantityNameInfoProvider::instance()->categoryFromQuantityName( s );
        EXPECT_TRUE( cat == RifEclipseSummaryAddress::SUMMARY_WELL );
    }
}

TEST( DISABLED_RiuSummaryQuantityNameInfoProvider, PerformanceLookup )
{
    std::vector<std::string> values;

    values.emplace_back( "WOPT" );
    values.emplace_back( "WOPR" );
    values.emplace_back( "FOPT" );
    values.emplace_back( "FOPR" );
    values.emplace_back( "BHP" );
    values.emplace_back( "nothing" );

    auto start = std::chrono::high_resolution_clock::now();

    const size_t iterationCount = 10000000;

    for ( size_t i = 0; i < iterationCount; i++ )
    {
        for ( const auto& s : values )
        {
            RiuSummaryQuantityNameInfoProvider::instance()->categoryFromQuantityName( s );
        }
    }

    auto                          end  = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "RiuSummaryQuantityNameInfoProvider : Duration " << std::setw( 9 ) << diff.count() << " s\n";
}
