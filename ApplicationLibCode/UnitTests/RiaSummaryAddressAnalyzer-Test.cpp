#include "gtest/gtest.h"

#include "Summary/RiaSummaryAddressAnalyzer.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaSummaryAddressAnalyzer, WellCompletions )
{
    std::vector<RifEclipseSummaryAddress> addresses;

    // Well A
    std::string wellNameA = "well_name_a";
    {
        RifEclipseSummaryAddress adr = RifEclipseSummaryAddress::wellConnectionAddress( "quantity_name", wellNameA, 1, 2, 3 );
        addresses.push_back( adr );
    }
    {
        RifEclipseSummaryAddress adr = RifEclipseSummaryAddress::wellConnectionAddress( "quantity_name", wellNameA, 1, 2, 3 );
        addresses.push_back( adr );
    }
    {
        RifEclipseSummaryAddress adr = RifEclipseSummaryAddress::wellConnectionAddress( "quantity_name", wellNameA, 5, 2, 3 );
        addresses.push_back( adr );
    }

    // Well B
    std::string wellNameB = "well_name_b";
    {
        RifEclipseSummaryAddress adr = RifEclipseSummaryAddress::wellConnectionAddress( "quantity_name", wellNameB, 5, 2, 3 );
        addresses.push_back( adr );
    }
    {
        RifEclipseSummaryAddress adr = RifEclipseSummaryAddress::wellConnectionAddress( "quantity_name", wellNameB, 5, 4, 3 );
        addresses.push_back( adr );
    }
    {
        RifEclipseSummaryAddress adr = RifEclipseSummaryAddress::wellConnectionAddress( "quantity_name", wellNameB, 5, 4, 30 );
        addresses.push_back( adr );
    }

    RiaSummaryAddressAnalyzer analyzer;
    analyzer.appendAddresses( addresses );

    EXPECT_EQ( 2u, analyzer.wellNames().size() );

    auto connectionsForA = analyzer.wellConnections( wellNameA );
    EXPECT_EQ( 2u, connectionsForA.size() );

    auto connectionsForB = analyzer.wellConnections( wellNameB );
    EXPECT_EQ( 3u, connectionsForB.size() );
    std::string tupleToFind = "5,4,30";
    EXPECT_TRUE( connectionsForB.find( tupleToFind ) != connectionsForB.end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaSummaryAddressAnalyzer, WellSegments )
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

    RiaSummaryAddressAnalyzer analyzer;
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
TEST( RiaSummaryAddressAnalyzer, CellBlocks )
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

    RiaSummaryAddressAnalyzer analyzer;
    analyzer.appendAddresses( addresses );

    auto blocks = analyzer.blocks();
    EXPECT_EQ( 3u, blocks.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaSummaryAddressAnalyzer, QuantitiesPerCategory )
{
    std::vector<RifEclipseSummaryAddress> addresses;

    {
        RifEclipseSummaryAddress adr = RifEclipseSummaryAddress::fieldAddress( "FOPT" );
        addresses.push_back( adr );
    }
    {
        RifEclipseSummaryAddress adr = RifEclipseSummaryAddress::fieldAddress( "FOPR" );
        addresses.push_back( adr );
    }

    {
        RifEclipseSummaryAddress adr = RifEclipseSummaryAddress::wellAddress( "WOPT", "WellA" );
        addresses.push_back( adr );
    }
    {
        RifEclipseSummaryAddress adr = RifEclipseSummaryAddress::wellAddress( "WOPR", "WellB" );
        addresses.push_back( adr );
    }
    {
        RifEclipseSummaryAddress adr = RifEclipseSummaryAddress::wellAddress( "WWPR", "WellA" );
        addresses.push_back( adr );
    }

    RiaSummaryAddressAnalyzer analyzer;
    analyzer.appendAddresses( addresses );

    auto categories = analyzer.categories();
    EXPECT_EQ( 2u, categories.size() );

    auto vectorNamesForWells = analyzer.vectorNamesForCategory( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL );
    EXPECT_EQ( 3u, vectorNamesForWells.size() );
}
