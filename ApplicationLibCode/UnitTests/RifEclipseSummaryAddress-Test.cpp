#include "gtest/gtest.h"

#include "RifEclipseSummaryAddress.h"
#include "RimSummaryAddress.h"

#include <QString>

#include <algorithm>
#include <random>
#include <string>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseSummaryAddressTest, TestEclipseAddressParsing_Field )
{
    std::string addrString = "FOPT";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddressParseErrorTokens( addrString );

    EXPECT_TRUE( addr.isValid() );
    EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_FIELD, addr.category() );
    EXPECT_EQ( "FOPT", addr.vectorName() );
    EXPECT_FALSE( addr.isErrorResult() );
}

TEST( RifEclipseSummaryAddressTest, TestEclipseAddressParsing_Aquifer )
{
    std::string addrString = "AAQR:456";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddressParseErrorTokens( addrString );

    EXPECT_TRUE( addr.isValid() );
    EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_AQUIFER, addr.category() );
    EXPECT_EQ( "AAQR", addr.vectorName() );
    EXPECT_EQ( 456, addr.aquiferNumber() );
    EXPECT_FALSE( addr.isErrorResult() );
}

TEST( RifEclipseSummaryAddressTest, TestEclipseAddressParsing_Network )
{
    std::string addrString = "NETW";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddressParseErrorTokens( addrString );

    EXPECT_TRUE( addr.isValid() );
    EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_NETWORK, addr.category() );
    EXPECT_EQ( "NETW", addr.vectorName() );
    EXPECT_FALSE( addr.isErrorResult() );
}

TEST( RifEclipseSummaryAddressTest, TestEclipseAddressParsing_Network_name )
{
    std::string addrString = "NETW:MYNAME";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddressParseErrorTokens( addrString );

    EXPECT_TRUE( addr.isValid() );
    EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_NETWORK, addr.category() );
    EXPECT_EQ( "NETW", addr.vectorName() );
    EXPECT_EQ( "MYNAME", addr.networkName() );
    EXPECT_FALSE( addr.isErrorResult() );
}

TEST( RifEclipseSummaryAddressTest, DISABLED_TestEclipseAddressParsing_Misc )
{
    std::string addrString = "CPU";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddressParseErrorTokens( addrString );

    EXPECT_TRUE( addr.isValid() );
    EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_MISC, addr.category() );
    EXPECT_EQ( "CPU", addr.vectorName() );
    EXPECT_FALSE( addr.isErrorResult() );
}

TEST( RifEclipseSummaryAddressTest, TestEclipseAddressParsing_Region )
{
    std::string addrString = "RPR:7081";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddressParseErrorTokens( addrString );

    EXPECT_TRUE( addr.isValid() );
    EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_REGION, addr.category() );
    EXPECT_EQ( "RPR", addr.vectorName() );
    EXPECT_EQ( 7081, addr.regionNumber() );
    EXPECT_FALSE( addr.isErrorResult() );
}

TEST( RifEclipseSummaryAddressTest, TestEclipseAddressParsing_RegionToRegion )
{
    std::string addrString = "ROFR:7081-8001";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddressParseErrorTokens( addrString );

    EXPECT_TRUE( addr.isValid() );
    EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_REGION_2_REGION, addr.category() );
    EXPECT_EQ( "ROFR", addr.vectorName() );
    EXPECT_EQ( 7081, addr.regionNumber() );
    EXPECT_EQ( 8001, addr.regionNumber2() );
    EXPECT_FALSE( addr.isErrorResult() );
}

TEST( RifEclipseSummaryAddressTest, TestEclipseAddressParsing_WellGroup )
{
    std::string addrString = "GOPR:WELLS1";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddressParseErrorTokens( addrString );

    EXPECT_TRUE( addr.isValid() );
    EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_GROUP, addr.category() );
    EXPECT_EQ( "GOPR", addr.vectorName() );
    EXPECT_EQ( "WELLS1", addr.groupName() );
    EXPECT_FALSE( addr.isErrorResult() );
}

TEST( RifEclipseSummaryAddressTest, TestEclipseAddressParsing_Well )
{
    std::string addrString = "WOPR:B-2H";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddressParseErrorTokens( addrString );

    EXPECT_TRUE( addr.isValid() );
    EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL, addr.category() );
    EXPECT_EQ( "WOPR", addr.vectorName() );
    EXPECT_EQ( "B-2H", addr.wellName() );
    EXPECT_FALSE( addr.isErrorResult() );
}

TEST( RifEclipseSummaryAddressTest, TestEclipseAddressParsing_WellConnection )
{
    std::string addrString = "CPRL:B-1H:15,13,14";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddressParseErrorTokens( addrString );

    EXPECT_TRUE( addr.isValid() );
    EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_CONNECTION, addr.category() );
    EXPECT_EQ( "CPRL", addr.vectorName() );
    EXPECT_EQ( "B-1H", addr.wellName() );
    EXPECT_EQ( 15, addr.cellI() );
    EXPECT_EQ( 13, addr.cellJ() );
    EXPECT_EQ( 14, addr.cellK() );
    EXPECT_FALSE( addr.isErrorResult() );
}

TEST( RifEclipseSummaryAddressTest, TestEclipseAddressParsing_WellCompletion )
{
    std::string addrString = "WGLRL:B-1H:15";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddressParseErrorTokens( addrString );

    EXPECT_TRUE( addr.isValid() );
    EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_COMPLETION, addr.category() );
    EXPECT_EQ( "WGLRL", addr.vectorName() );
    EXPECT_EQ( "B-1H", addr.wellName() );
    EXPECT_EQ( 15, addr.wellCompletionNumber() );
    EXPECT_FALSE( addr.isErrorResult() );
}

TEST( RifEclipseSummaryAddressTest, TestEclipseAddressParsing_WellLgr )
{
    std::string addrString = "LWABC:LGRNA:B-10H";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddressParseErrorTokens( addrString );

    EXPECT_TRUE( addr.isValid() );
    EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_LGR, addr.category() );
    EXPECT_EQ( "LWABC", addr.vectorName() );
    EXPECT_EQ( "LGRNA", addr.lgrName() );
    EXPECT_EQ( "B-10H", addr.wellName() );
    EXPECT_FALSE( addr.isErrorResult() );
}

TEST( RifEclipseSummaryAddressTest, TestEclipseAddressParsing_WellCompletionLgr )
{
    std::string addrString = "LCGAS:LGR1:B-1H:11,12,13";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddressParseErrorTokens( addrString );

    EXPECT_TRUE( addr.isValid() );
    EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_CONNECTION_LGR, addr.category() );
    EXPECT_EQ( "LCGAS", addr.vectorName() );
    EXPECT_EQ( "LGR1", addr.lgrName() );
    EXPECT_EQ( "B-1H", addr.wellName() );
    EXPECT_EQ( 11, addr.cellI() );
    EXPECT_EQ( 12, addr.cellJ() );
    EXPECT_EQ( 13, addr.cellK() );
    EXPECT_FALSE( addr.isErrorResult() );
}

TEST( RifEclipseSummaryAddressTest, TestEclipseAddressParsing_WellSegment )
{
    std::string addrString = "SOFR:B-5H:32";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddressParseErrorTokens( addrString );

    EXPECT_TRUE( addr.isValid() );
    EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_SEGMENT, addr.category() );
    EXPECT_EQ( "SOFR", addr.vectorName() );
    EXPECT_EQ( "B-5H", addr.wellName() );
    EXPECT_EQ( 32, addr.wellSegmentNumber() );
    EXPECT_FALSE( addr.isErrorResult() );
}

TEST( RifEclipseSummaryAddressTest, TestEclipseAddressParsing_Block )
{
    std::string addrString = "BPR:123,122,121";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddressParseErrorTokens( addrString );

    EXPECT_TRUE( addr.isValid() );
    EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_BLOCK, addr.category() );
    EXPECT_EQ( "BPR", addr.vectorName() );
    EXPECT_EQ( 123, addr.cellI() );
    EXPECT_EQ( 122, addr.cellJ() );
    EXPECT_EQ( 121, addr.cellK() );
    EXPECT_FALSE( addr.isErrorResult() );
}

TEST( RifEclipseSummaryAddressTest, TestEclipseAddressParsing_BlockLgr )
{
    std::string addrString = "LBABC:LGRN:45,47,49";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddressParseErrorTokens( addrString );

    EXPECT_TRUE( addr.isValid() );
    EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_BLOCK_LGR, addr.category() );
    EXPECT_EQ( "LBABC", addr.vectorName() );
    EXPECT_EQ( "LGRN", addr.lgrName() );
    EXPECT_EQ( 45, addr.cellI() );
    EXPECT_EQ( 47, addr.cellJ() );
    EXPECT_EQ( 49, addr.cellK() );
    EXPECT_FALSE( addr.isErrorResult() );
}

TEST( RifEclipseSummaryAddressTest, TestEclipseAddressParsing_Imported )
{
    std::string addrString = "FAULT (Imp)";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddressParseErrorTokens( addrString );

    EXPECT_TRUE( addr.isValid() );
    EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_IMPORTED, addr.category() );
    EXPECT_EQ( "FAULT (Imp)", addr.vectorName() );
    EXPECT_FALSE( addr.isErrorResult() );
}

TEST( RifEclipseSummaryAddressTest, TestEclipseAddressParsing_ErrorResult1 )
{
    std::string addrString = "ER:AAQR:456";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddressParseErrorTokens( addrString );

    EXPECT_TRUE( addr.isValid() );
    EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_AQUIFER, addr.category() );
    EXPECT_EQ( "AAQR", addr.vectorName() );
    EXPECT_EQ( 456, addr.aquiferNumber() );
    EXPECT_TRUE( addr.isErrorResult() );
}

TEST( RifEclipseSummaryAddressTest, TestEclipseAddressParsing_ErrorResult2 )
{
    std::string addrString = "ERR:LCGAS:LGR1:B-1H:11,12,13";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddressParseErrorTokens( addrString );

    EXPECT_TRUE( addr.isValid() );
    EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_CONNECTION_LGR, addr.category() );
    EXPECT_EQ( "LCGAS", addr.vectorName() );
    EXPECT_EQ( "LGR1", addr.lgrName() );
    EXPECT_EQ( "B-1H", addr.wellName() );
    EXPECT_EQ( 11, addr.cellI() );
    EXPECT_EQ( 12, addr.cellJ() );
    EXPECT_EQ( 13, addr.cellK() );
    EXPECT_TRUE( addr.isErrorResult() );
}

TEST( RifEclipseSummaryAddressTest, TestEclipseAddressParsing_ErrorResult3 )
{
    std::string addrString = "ERROR:FAULT (Imp)";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddressParseErrorTokens( addrString );

    EXPECT_TRUE( addr.isValid() );
    EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_IMPORTED, addr.category() );
    EXPECT_EQ( "FAULT (Imp)", addr.vectorName() );
    EXPECT_TRUE( addr.isErrorResult() );
}

TEST( RifEclipseSummaryAddressTest, TestEclipseAddressIjkParsing )
{
    RifEclipseSummaryAddressDefines::SummaryCategory cat = RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_CONNECTION;
    std::map<RifEclipseSummaryAddressDefines::SummaryIdentifierType, std::string> identifiers( {
        { RifEclipseSummaryAddressDefines::SummaryIdentifierType::INPUT_WELL_NAME, "1-BH" },
        { RifEclipseSummaryAddressDefines::SummaryIdentifierType::INPUT_CELL_IJK, "6, 7, 8" },
        { RifEclipseSummaryAddressDefines::SummaryIdentifierType::INPUT_VECTOR_NAME, "WOPR" },
    } );

    RifEclipseSummaryAddress addr( cat, identifiers );

    EXPECT_TRUE( addr.isValid() );
    EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_CONNECTION, addr.category() );
    EXPECT_EQ( "WOPR", addr.vectorName() );
    EXPECT_EQ( "1-BH", addr.wellName() );
    EXPECT_EQ( 6, addr.cellI() );
    EXPECT_EQ( 7, addr.cellJ() );
    EXPECT_EQ( 8, addr.cellK() );
    EXPECT_TRUE( !addr.isErrorResult() );
}

TEST( RifEclipseSummaryAddressTest, TestEclipseAddressRegToRegParsing )
{
    RifEclipseSummaryAddressDefines::SummaryCategory cat = RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_REGION_2_REGION;
    std::map<RifEclipseSummaryAddressDefines::SummaryIdentifierType, std::string> identifiers( {
        { RifEclipseSummaryAddressDefines::SummaryIdentifierType::INPUT_REGION_2_REGION, "123 - 456" },
        { RifEclipseSummaryAddressDefines::SummaryIdentifierType::INPUT_VECTOR_NAME, "ROFR" },
    } );

    RifEclipseSummaryAddress addr( cat, identifiers );

    EXPECT_TRUE( addr.isValid() );
    EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_REGION_2_REGION, addr.category() );
    EXPECT_EQ( "ROFR", addr.vectorName() );
    EXPECT_EQ( 123, addr.regionNumber() );
    EXPECT_EQ( 456, addr.regionNumber2() );
    EXPECT_TRUE( !addr.isErrorResult() );
}

TEST( RifEclipseSummaryAddressTest, TestQuantityNameManipulations )
{
    {
        auto s = RifEclipseSummaryAddress::baseVectorName( "FOPT" );
        EXPECT_EQ( "FOPT", s );
    }

    {
        auto s = RifEclipseSummaryAddress::baseVectorName( "FOPT_1" );
        EXPECT_EQ( "FOPT", s );
    }

    {
        auto s = RifEclipseSummaryAddress::baseVectorName( "FOPR" );
        EXPECT_EQ( "FOPR", s );
    }

    {
        auto s = RifEclipseSummaryAddress::baseVectorName( "FOPR_1" );
        EXPECT_EQ( "FOPR", s );
    }

    {
        // https://github.com/OPM/ResInsight/issues/6481
        auto s = RifEclipseSummaryAddress::baseVectorName( "FCMIT_1" );
        EXPECT_EQ( "FCMIT", s );
    }
}

TEST( RifEclipseSummaryAddressTest, LogicalOperators )
{
    // The <=> operator will compare individual members variables by the order of their declaration.

    auto field_a = RifEclipseSummaryAddress::fieldAddress( "A" );
    auto field_b = RifEclipseSummaryAddress::fieldAddress( "B" );
    auto field_c = RifEclipseSummaryAddress::fieldAddress( "C" );

    // Sort by vector name, then by number
    auto aq_a = RifEclipseSummaryAddress::aquiferAddress( "A", 1 );
    auto aq_b = RifEclipseSummaryAddress::aquiferAddress( "B", 1 );
    auto aq_c = RifEclipseSummaryAddress::aquiferAddress( "A", 2 );

    // Sort order is K, J, I
    auto block_a = RifEclipseSummaryAddress::blockAddress( "A", 1, 1, 1 );
    auto block_b = RifEclipseSummaryAddress::blockAddress( "A", 2, 1, 1 );
    auto block_c = RifEclipseSummaryAddress::blockAddress( "A", 1, 1, 2 );
    auto block_d = RifEclipseSummaryAddress::blockAddress( "A", 2, 1, 2 );

    std::vector<RifEclipseSummaryAddress> addresses;

    addresses.push_back( field_a );
    addresses.push_back( field_b );
    addresses.push_back( field_c );

    addresses.push_back( aq_a );
    addresses.push_back( aq_b );
    addresses.push_back( aq_c );

    addresses.push_back( block_a );
    addresses.push_back( block_b );
    addresses.push_back( block_c );
    addresses.push_back( block_d );

    std::random_device rd;
    std::mt19937       g( rd() );
    std::shuffle( addresses.begin(), addresses.end(), g );
    std::sort( addresses.begin(), addresses.end() );

    EXPECT_TRUE( addresses.at( 0 ) == field_a );
    EXPECT_TRUE( addresses.at( 1 ) == field_b );
    EXPECT_TRUE( addresses.at( 2 ) == field_c );

    EXPECT_TRUE( addresses.at( 3 ) == aq_a );
    EXPECT_TRUE( addresses.at( 4 ) == aq_c );
    EXPECT_TRUE( addresses.at( 5 ) == aq_b );

    EXPECT_TRUE( addresses.at( 6 ) == block_a );
    EXPECT_TRUE( addresses.at( 7 ) == block_b );
    EXPECT_TRUE( addresses.at( 8 ) == block_c );
    EXPECT_TRUE( addresses.at( 9 ) == block_d );
}

TEST( RifEclipseSummaryAddressTest, ConversionFromTextToAddress )
{
    // Aquifer
    {
        auto adr = RifEclipseSummaryAddress::fromEclipseTextAddress( "AAQR:456" );
        EXPECT_TRUE( adr.isValid() );
        EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_AQUIFER, adr.category() );
        EXPECT_EQ( "AAQR", adr.vectorName() );
        EXPECT_EQ( 456, adr.aquiferNumber() );
    }

    // Unknown aquifer
    {
        auto adr = RifEclipseSummaryAddress::fromEclipseTextAddress( "AAQCEE:2" );
        EXPECT_TRUE( adr.isValid() );
        EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_AQUIFER, adr.category() );
        EXPECT_EQ( "AAQCEE", adr.vectorName() );
        EXPECT_EQ( 2, adr.aquiferNumber() );
    }

    // Block
    {
        auto adr = RifEclipseSummaryAddress::fromEclipseTextAddress( "BPR:123,122,121" );
        EXPECT_TRUE( adr.isValid() );
        EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_BLOCK, adr.category() );
        EXPECT_EQ( "BPR", adr.vectorName() );
        EXPECT_EQ( 123, adr.cellI() );
        EXPECT_EQ( 122, adr.cellJ() );
        EXPECT_EQ( 121, adr.cellK() );
    }

    // Unknown block
    {
        auto adr = RifEclipseSummaryAddress::fromEclipseTextAddress( "BPGTEST:1,2,3" );
        EXPECT_TRUE( adr.isValid() );
        EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_BLOCK, adr.category() );
        EXPECT_EQ( "BPGTEST", adr.vectorName() );
        EXPECT_EQ( 1, adr.cellI() );
        EXPECT_EQ( 2, adr.cellJ() );
        EXPECT_EQ( 3, adr.cellK() );
    }

    // Well completion
    {
        auto adr = RifEclipseSummaryAddress::fromEclipseTextAddress( "WGLRL:B-1H:15" );
        EXPECT_TRUE( adr.isValid() );
        EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_COMPLETION, adr.category() );
        EXPECT_EQ( "WGLRL", adr.vectorName() );
        EXPECT_EQ( "B-1H", adr.wellName() );
        EXPECT_EQ( 15, adr.wellCompletionNumber() );
    }

    // Unknown well completion
    {
        auto adr = RifEclipseSummaryAddress::fromEclipseTextAddress( "WGLRTESAT:B-1H:15" );
        EXPECT_TRUE( adr.isValid() );
        EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_COMPLETION, adr.category() );
        EXPECT_EQ( "WGLRTESAT", adr.vectorName() );
        EXPECT_EQ( "B-1H", adr.wellName() );
        EXPECT_EQ( 15, adr.wellCompletionNumber() );
    }

    // Well connection
    {
        auto adr = RifEclipseSummaryAddress::fromEclipseTextAddress( "CPRL:B-1H:15,13,14" );
        EXPECT_TRUE( adr.isValid() );
        EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_CONNECTION, adr.category() );
        EXPECT_EQ( "CPRL", adr.vectorName() );
        EXPECT_EQ( "B-1H", adr.wellName() );
        EXPECT_EQ( 15, adr.cellI() );
        EXPECT_EQ( 13, adr.cellJ() );
        EXPECT_EQ( 14, adr.cellK() );
    }

    // Unknown well connection
    {
        auto adr = RifEclipseSummaryAddress::fromEclipseTextAddress( "CAPIRSS:B-1H:15,13,22" );
        EXPECT_TRUE( adr.isValid() );
        EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_CONNECTION, adr.category() );
        EXPECT_EQ( "CAPIRSS", adr.vectorName() );
        EXPECT_EQ( "B-1H", adr.wellName() );
        EXPECT_EQ( 15, adr.cellI() );
        EXPECT_EQ( 13, adr.cellJ() );
        EXPECT_EQ( 22, adr.cellK() );
    }

    // Field
    {
        auto adr = RifEclipseSummaryAddress::fromEclipseTextAddress( "FOPT" );
        EXPECT_TRUE( adr.isValid() );
        EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_FIELD, adr.category() );
        EXPECT_EQ( "FOPT", adr.vectorName() );
    }

    // Unknown field
    {
        auto adr = RifEclipseSummaryAddress::fromEclipseTextAddress( "FOPCEE" );
        EXPECT_TRUE( adr.isValid() );
        EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_FIELD, adr.category() );
        EXPECT_EQ( "FOPCEE", adr.vectorName() );
    }

    // Group
    {
        auto adr = RifEclipseSummaryAddress::fromEclipseTextAddress( "GOPR:WELLS1" );
        EXPECT_TRUE( adr.isValid() );
        EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_GROUP, adr.category() );
        EXPECT_EQ( "GOPR", adr.vectorName() );
        EXPECT_EQ( "WELLS1", adr.groupName() );
    }

    // Unknown group
    {
        auto adr = RifEclipseSummaryAddress::fromEclipseTextAddress( "GOPRTEST:WELLS1" );
        EXPECT_TRUE( adr.isValid() );
        EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_GROUP, adr.category() );
        EXPECT_EQ( "GOPRTEST", adr.vectorName() );
        EXPECT_EQ( "WELLS1", adr.groupName() );
    }

    // Region
    {
        auto adr = RifEclipseSummaryAddress::fromEclipseTextAddress( "RPR:7081" );
        EXPECT_TRUE( adr.isValid() );
        EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_REGION, adr.category() );
        EXPECT_EQ( "RPR", adr.vectorName() );
        EXPECT_EQ( 7081, adr.regionNumber() );
    }

    // Unknown region
    {
        auto adr = RifEclipseSummaryAddress::fromEclipseTextAddress( "RPRTEST:7081" );
        EXPECT_TRUE( adr.isValid() );
        EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_REGION, adr.category() );
        EXPECT_EQ( "RPRTEST", adr.vectorName() );
        EXPECT_EQ( 7081, adr.regionNumber() );
    }

    // Region to region
    {
        auto adr = RifEclipseSummaryAddress::fromEclipseTextAddress( "RGFT:7081-8001" );
        EXPECT_TRUE( adr.isValid() );
        EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_REGION_2_REGION, adr.category() );
        EXPECT_EQ( "RGFT", adr.vectorName() );
        EXPECT_EQ( 7081, adr.regionNumber() );
        EXPECT_EQ( 8001, adr.regionNumber2() );
    }

    // Unknown region to region is skipped, no general rule to identify region to region

    // Segment
    {
        auto adr = RifEclipseSummaryAddress::fromEclipseTextAddress( "SOFR:B-5H:32" );
        EXPECT_TRUE( adr.isValid() );
        EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_SEGMENT, adr.category() );
        EXPECT_EQ( "SOFR", adr.vectorName() );
        EXPECT_EQ( "B-5H", adr.wellName() );
        EXPECT_EQ( 32, adr.wellSegmentNumber() );
    }

    // Unknown segment
    {
        auto adr = RifEclipseSummaryAddress::fromEclipseTextAddress( "SOFRTEST:B-5H:32" );
        EXPECT_TRUE( adr.isValid() );
        EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_SEGMENT, adr.category() );
        EXPECT_EQ( "SOFRTEST", adr.vectorName() );
        EXPECT_EQ( "B-5H", adr.wellName() );
        EXPECT_EQ( 32, adr.wellSegmentNumber() );
    }

    // Well
    {
        auto adr = RifEclipseSummaryAddress::fromEclipseTextAddress( "WOPR:B-2H" );
        EXPECT_TRUE( adr.isValid() );
        EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL, adr.category() );
        EXPECT_EQ( "WOPR", adr.vectorName() );
        EXPECT_EQ( "B-2H", adr.wellName() );
    }

    // Unknown well
    {
        auto adr = RifEclipseSummaryAddress::fromEclipseTextAddress( "WOPRTEST:B-2H" );
        EXPECT_TRUE( adr.isValid() );
        EXPECT_EQ( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL, adr.category() );
        EXPECT_EQ( "WOPRTEST", adr.vectorName() );
        EXPECT_EQ( "B-2H", adr.wellName() );
    }
}

TEST( RifEclipseSummaryAddressTest, Conversion_RifEclipseSummaryAddress_RimSummaryAddress_RoundTrip )
{
    // Create a RifEclipseSummaryAddress with various fields set
    RifEclipseSummaryAddress original = RifEclipseSummaryAddress::wellConnectionAddress( "WOPR", "B-2H", 10, 20, 30 );
    original.setStatisticsType( RifEclipseSummaryAddressDefines::StatisticsType::P50 );
    original.setAsErrorResult();
    original.setId( 42 );

    // Convert to RimSummaryAddress
    RimSummaryAddress rimAddr;
    rimAddr.setAddress( original );

    // Convert back to RifEclipseSummaryAddress
    RifEclipseSummaryAddress roundTrip = rimAddr.address();

    // Check that all relevant fields are preserved
    EXPECT_TRUE( roundTrip.isValid() );
    EXPECT_EQ( original.category(), roundTrip.category() );
    EXPECT_EQ( original.vectorName(), roundTrip.vectorName() );
    EXPECT_EQ( original.wellName(), roundTrip.wellName() );
    EXPECT_EQ( original.cellI(), roundTrip.cellI() );
    EXPECT_EQ( original.cellJ(), roundTrip.cellJ() );
    EXPECT_EQ( original.cellK(), roundTrip.cellK() );
    EXPECT_EQ( original.statisticsType(), roundTrip.statisticsType() );
    EXPECT_EQ( original.isErrorResult(), roundTrip.isErrorResult() );
    EXPECT_EQ( original.id(), roundTrip.id() );
}
