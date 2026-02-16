#include "gtest/gtest.h"

#include "RifEclipseSummaryAddress.h"
#include "RifEclipseSummaryAddressDefines.h"
#include "RimSummaryCalculation.h"

using SummaryCategory = RifEclipseSummaryAddressDefines::SummaryCategory;

//--------------------------------------------------------------------------------------------------
/// Test subclass that exposes protected methods for unit testing
//--------------------------------------------------------------------------------------------------
class RimSummaryCalculationTester : public RimSummaryCalculation
{
public:
    RimSummaryCalculationTester()
    {
        setExpression( "CALC := a + b" );
        setId( 42 );
    }

    std::vector<RimSummaryCalculationAddress> testAllAddressesForCategory( SummaryCategory                          category,
                                                                           const std::set<RifEclipseSummaryAddress>& allResultAddresses ) const
    {
        return allAddressesForCategory( category, allResultAddresses );
    }

    RimSummaryCalculationAddress testSingleAddressesForCategory( const RifEclipseSummaryAddress& address ) const
    {
        return singleAddressesForCategory( address );
    }
};

//--------------------------------------------------------------------------------------------------
// allAddressesForCategory - SUMMARY_BLOCK
//--------------------------------------------------------------------------------------------------

TEST( RimSummaryCalculation, Block_UniqueBlocks )
{
    RimSummaryCalculationTester calc;

    std::set<RifEclipseSummaryAddress> addresses;
    addresses.insert( RifEclipseSummaryAddress::blockAddress( "BPR", 1, 2, 3 ) );
    addresses.insert( RifEclipseSummaryAddress::blockAddress( "BPR", 1, 2, 3 ) ); // duplicate
    addresses.insert( RifEclipseSummaryAddress::blockAddress( "BPR", 4, 5, 6 ) );
    addresses.insert( RifEclipseSummaryAddress::blockAddress( "BPR", 7, 8, 9 ) );

    auto result = calc.testAllAddressesForCategory( SummaryCategory::SUMMARY_BLOCK, addresses );

    EXPECT_EQ( 3u, result.size() );
}

TEST( RimSummaryCalculation, Block_EmptyInput )
{
    RimSummaryCalculationTester calc;

    std::set<RifEclipseSummaryAddress> addresses;

    auto result = calc.testAllAddressesForCategory( SummaryCategory::SUMMARY_BLOCK, addresses );

    EXPECT_TRUE( result.empty() );
}

TEST( RimSummaryCalculation, Block_IgnoresOtherCategories )
{
    RimSummaryCalculationTester calc;

    std::set<RifEclipseSummaryAddress> addresses;
    addresses.insert( RifEclipseSummaryAddress::wellAddress( "WOPR", "W1" ) );
    addresses.insert( RifEclipseSummaryAddress::wellAddress( "WOPR", "W2" ) );

    auto result = calc.testAllAddressesForCategory( SummaryCategory::SUMMARY_BLOCK, addresses );

    EXPECT_TRUE( result.empty() );
}

TEST( RimSummaryCalculation, Block_SingleBlock )
{
    RimSummaryCalculationTester calc;

    std::set<RifEclipseSummaryAddress> addresses;
    addresses.insert( RifEclipseSummaryAddress::blockAddress( "BPR", 10, 20, 30 ) );

    auto result = calc.testAllAddressesForCategory( SummaryCategory::SUMMARY_BLOCK, addresses );

    ASSERT_EQ( 1u, result.size() );
    EXPECT_EQ( 10, result[0].address().cellI() );
    EXPECT_EQ( 20, result[0].address().cellJ() );
    EXPECT_EQ( 30, result[0].address().cellK() );
    EXPECT_EQ( "CALC", result[0].address().vectorName() );
    EXPECT_EQ( 42, result[0].address().id() );
}

//--------------------------------------------------------------------------------------------------
// allAddressesForCategory - SUMMARY_BLOCK_LGR
//--------------------------------------------------------------------------------------------------

TEST( RimSummaryCalculation, BlockLgr_MultiLgrMultiBlock )
{
    RimSummaryCalculationTester calc;

    std::set<RifEclipseSummaryAddress> addresses;
    addresses.insert( RifEclipseSummaryAddress::blockLgrAddress( "LBPR", "LGR1", 1, 1, 1 ) );
    addresses.insert( RifEclipseSummaryAddress::blockLgrAddress( "LBPR", "LGR1", 2, 2, 2 ) );
    addresses.insert( RifEclipseSummaryAddress::blockLgrAddress( "LBPR", "LGR2", 3, 3, 3 ) );
    addresses.insert( RifEclipseSummaryAddress::blockLgrAddress( "LBPR", "LGR2", 4, 4, 4 ) );

    auto result = calc.testAllAddressesForCategory( SummaryCategory::SUMMARY_BLOCK_LGR, addresses );

    EXPECT_EQ( 4u, result.size() );
}

TEST( RimSummaryCalculation, BlockLgr_DeduplicatesWithinLgr )
{
    RimSummaryCalculationTester calc;

    std::set<RifEclipseSummaryAddress> addresses;
    addresses.insert( RifEclipseSummaryAddress::blockLgrAddress( "LBPR", "LGR1", 1, 1, 1 ) );
    addresses.insert( RifEclipseSummaryAddress::blockLgrAddress( "LBPR", "LGR1", 1, 1, 1 ) ); // duplicate
    addresses.insert( RifEclipseSummaryAddress::blockLgrAddress( "LBPR", "LGR1", 2, 2, 2 ) );

    auto result = calc.testAllAddressesForCategory( SummaryCategory::SUMMARY_BLOCK_LGR, addresses );

    EXPECT_EQ( 2u, result.size() );
}

TEST( RimSummaryCalculation, BlockLgr_IgnoresOtherCategories )
{
    RimSummaryCalculationTester calc;

    std::set<RifEclipseSummaryAddress> addresses;
    addresses.insert( RifEclipseSummaryAddress::blockAddress( "BPR", 1, 2, 3 ) );
    addresses.insert( RifEclipseSummaryAddress::wellAddress( "WOPR", "W1" ) );
    addresses.insert( RifEclipseSummaryAddress::blockLgrAddress( "LBPR", "LGR1", 5, 6, 7 ) );

    auto result = calc.testAllAddressesForCategory( SummaryCategory::SUMMARY_BLOCK_LGR, addresses );

    ASSERT_EQ( 1u, result.size() );
    EXPECT_EQ( "LGR1", result[0].address().lgrName() );
    EXPECT_EQ( 5, result[0].address().cellI() );
    EXPECT_EQ( 6, result[0].address().cellJ() );
    EXPECT_EQ( 7, result[0].address().cellK() );
}

//--------------------------------------------------------------------------------------------------
// singleAddressesForCategory
//--------------------------------------------------------------------------------------------------

TEST( RimSummaryCalculation, SingleAddress_Block )
{
    RimSummaryCalculationTester calc;

    auto input  = RifEclipseSummaryAddress::blockAddress( "BPR", 10, 20, 30 );
    auto result = calc.testSingleAddressesForCategory( input );

    EXPECT_EQ( "CALC", result.address().vectorName() );
    EXPECT_EQ( 42, result.address().id() );
    EXPECT_EQ( 10, result.address().cellI() );
    EXPECT_EQ( 20, result.address().cellJ() );
    EXPECT_EQ( 30, result.address().cellK() );
    EXPECT_EQ( SummaryCategory::SUMMARY_BLOCK, result.address().category() );
}

TEST( RimSummaryCalculation, SingleAddress_BlockLgr )
{
    RimSummaryCalculationTester calc;

    auto input  = RifEclipseSummaryAddress::blockLgrAddress( "LBPR", "LGR1", 5, 6, 7 );
    auto result = calc.testSingleAddressesForCategory( input );

    EXPECT_EQ( "CALC", result.address().vectorName() );
    EXPECT_EQ( 42, result.address().id() );
    EXPECT_EQ( "LGR1", result.address().lgrName() );
    EXPECT_EQ( 5, result.address().cellI() );
    EXPECT_EQ( 6, result.address().cellJ() );
    EXPECT_EQ( 7, result.address().cellK() );
    EXPECT_EQ( SummaryCategory::SUMMARY_BLOCK_LGR, result.address().category() );
}

//--------------------------------------------------------------------------------------------------
// substituteVariables
//--------------------------------------------------------------------------------------------------

TEST( RimSummaryCalculation, SubstituteVariables_Block )
{
    // Set up variables with a SUMMARY_BLOCK address at (1,2,3)
    SummaryCalculationVariable var;
    var.name           = "a";
    var.summaryCase    = nullptr;
    var.summaryAddress = RifEclipseSummaryAddress::blockAddress( "BPR", 1, 2, 3 );

    std::vector<SummaryCalculationVariable> vars = { var };

    // Substitute to (10,20,30)
    auto newAddress = RifEclipseSummaryAddress::blockAddress( "BPR", 10, 20, 30 );
    RimSummaryCalculation::substituteVariables( vars, newAddress );

    EXPECT_EQ( 10, vars[0].summaryAddress.cellI() );
    EXPECT_EQ( 20, vars[0].summaryAddress.cellJ() );
    EXPECT_EQ( 30, vars[0].summaryAddress.cellK() );
}

TEST( RimSummaryCalculation, SubstituteVariables_BlockLgr )
{
    // Set up variables with a SUMMARY_BLOCK_LGR address
    SummaryCalculationVariable var;
    var.name           = "a";
    var.summaryCase    = nullptr;
    var.summaryAddress = RifEclipseSummaryAddress::blockLgrAddress( "LBPR", "LGR1", 1, 2, 3 );

    std::vector<SummaryCalculationVariable> vars = { var };

    // Substitute to new block coordinates - validates the SUMMARY_BLOCK_LGR bug fix
    auto newAddress = RifEclipseSummaryAddress::blockLgrAddress( "LBPR", "LGR1", 10, 20, 30 );
    RimSummaryCalculation::substituteVariables( vars, newAddress );

    EXPECT_EQ( 10, vars[0].summaryAddress.cellI() );
    EXPECT_EQ( 20, vars[0].summaryAddress.cellJ() );
    EXPECT_EQ( 30, vars[0].summaryAddress.cellK() );
}
