#include "gtest/gtest.h"

#include "RiaSummaryCurveAnalyzer.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaSummaryCurveAnalyzer, WellCompletions )
{
    std::vector<RifEclipseSummaryAddress> addresses;

    // Well A
    std::string wellNameA = "well_name_a";
    {
        RifEclipseSummaryAddress adr =
            RifEclipseSummaryAddress::wellCompletionAddress( "quantity_name", wellNameA, 1, 2, 3 );
        addresses.push_back( adr );
    }
    {
        RifEclipseSummaryAddress adr =
            RifEclipseSummaryAddress::wellCompletionAddress( "quantity_name", wellNameA, 1, 2, 3 );
        addresses.push_back( adr );
    }
    {
        RifEclipseSummaryAddress adr =
            RifEclipseSummaryAddress::wellCompletionAddress( "quantity_name", wellNameA, 5, 2, 3 );
        addresses.push_back( adr );
    }

    // Well B
    std::string wellNameB = "well_name_b";
    {
        RifEclipseSummaryAddress adr =
            RifEclipseSummaryAddress::wellCompletionAddress( "quantity_name", wellNameB, 5, 2, 3 );
        addresses.push_back( adr );
    }
    {
        RifEclipseSummaryAddress adr =
            RifEclipseSummaryAddress::wellCompletionAddress( "quantity_name", wellNameB, 5, 4, 3 );
        addresses.push_back( adr );
    }
    {
        RifEclipseSummaryAddress adr =
            RifEclipseSummaryAddress::wellCompletionAddress( "quantity_name", wellNameB, 5, 4, 30 );
        addresses.push_back( adr );
    }

    RiaSummaryCurveAnalyzer analyzer;
    analyzer.appendAddresses( addresses );

    EXPECT_EQ( 2u, analyzer.wellNames().size() );

    auto completionsForA = analyzer.wellCompletions( wellNameA );
    EXPECT_EQ( 2u, completionsForA.size() );

    auto completionsForB = analyzer.wellCompletions( wellNameB );
    EXPECT_EQ( 3u, completionsForB.size() );
    std::string tupleToFind = "5, 4, 30";
    EXPECT_TRUE( completionsForB.find( tupleToFind ) != completionsForB.end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaSummaryCurveAnalyzer, WellSegments )
{
    std::vector<RifEclipseSummaryAddress> addresses;

    // Well A
    std::string wellNameA = "well_name_a";
    {
        RifEclipseSummaryAddress adr = RifEclipseSummaryAddress::wellSegmentAddress( "quantity_name", wellNameA, 1 );
        addresses.push_back( adr );
    }
    {
        RifEclipseSummaryAddress adr = RifEclipseSummaryAddress::wellSegmentAddress( "quantity_name", wellNameA, 1 );
        addresses.push_back( adr );
    }
    {
        RifEclipseSummaryAddress adr = RifEclipseSummaryAddress::wellSegmentAddress( "quantity_name", wellNameA, 30 );
        addresses.push_back( adr );
    }

    // Well B
    std::string wellNameB = "well_name_b";
    {
        RifEclipseSummaryAddress adr = RifEclipseSummaryAddress::wellSegmentAddress( "quantity_name", wellNameB, 1 );
        addresses.push_back( adr );
    }
    {
        RifEclipseSummaryAddress adr = RifEclipseSummaryAddress::wellSegmentAddress( "quantity_name", wellNameB, 3 );
        addresses.push_back( adr );
    }
    {
        RifEclipseSummaryAddress adr = RifEclipseSummaryAddress::wellSegmentAddress( "quantity_name", wellNameB, 10 );
        addresses.push_back( adr );
    }

    RiaSummaryCurveAnalyzer analyzer;
    analyzer.appendAddresses( addresses );

    EXPECT_EQ( 2u, analyzer.wellNames().size() );

    auto segmentsForA = analyzer.wellSegmentNumbers( wellNameA );
    EXPECT_EQ( 2u, segmentsForA.size() );

    auto segmentsForB = analyzer.wellSegmentNumbers( wellNameB );
    EXPECT_EQ( 3u, segmentsForB.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaSummaryCurveAnalyzer, CellBlocks )
{
    std::vector<RifEclipseSummaryAddress> addresses;

    {
        RifEclipseSummaryAddress adr = RifEclipseSummaryAddress::blockAddress( "quantity_name", 1, 2, 3 );
        addresses.push_back( adr );
    }
    {
        RifEclipseSummaryAddress adr = RifEclipseSummaryAddress::blockAddress( "quantity_name", 1, 2, 3 );
        addresses.push_back( adr );
    }
    {
        RifEclipseSummaryAddress adr = RifEclipseSummaryAddress::blockAddress( "quantity_name", 2, 2, 3 );
        addresses.push_back( adr );
    }
    {
        RifEclipseSummaryAddress adr = RifEclipseSummaryAddress::blockAddress( "quantity_name", 5, 2, 3 );
        addresses.push_back( adr );
    }

    RiaSummaryCurveAnalyzer analyzer;
    analyzer.appendAddresses( addresses );

    auto blocks = analyzer.blocks();
    EXPECT_EQ( 3u, blocks.size() );
}
