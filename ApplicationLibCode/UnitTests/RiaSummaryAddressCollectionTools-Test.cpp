#include "gtest/gtest.h"

#include "Summary/RiaSummaryAddressCollectionTools.h"
#include "Summary/RiaSummaryCurveDefinition.h"

#include "RifEclipseSummaryAddress.h"
#include "RimMockSummaryCase.h"

//--------------------------------------------------------------------------------------------------
// objectIdentifierForAddress tests
//--------------------------------------------------------------------------------------------------
TEST( RiaSummaryAddressCollectionTools, ObjectIdentifierForWellAddress )
{
    auto address = RifEclipseSummaryAddress::wellAddress( "WOPT", "WELL-1" );
    auto result =
        RiaSummaryAddressCollectionTools::objectIdentifierForAddress( address, RimSummaryAddressCollection::CollectionContentType::WELL );
    EXPECT_EQ( "WELL-1", result );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaSummaryAddressCollectionTools, ObjectIdentifierForGroupAddress )
{
    auto address = RifEclipseSummaryAddress::groupAddress( "GOPT", "GROUP-1" );
    auto result =
        RiaSummaryAddressCollectionTools::objectIdentifierForAddress( address, RimSummaryAddressCollection::CollectionContentType::GROUP );
    EXPECT_EQ( "GROUP-1", result );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaSummaryAddressCollectionTools, ObjectIdentifierForNetworkAddress )
{
    auto address = RifEclipseSummaryAddress::networkAddress( "NOPT", "NET-1" );
    auto result =
        RiaSummaryAddressCollectionTools::objectIdentifierForAddress( address, RimSummaryAddressCollection::CollectionContentType::NETWORK );
    EXPECT_EQ( "NET-1", result );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaSummaryAddressCollectionTools, ObjectIdentifierForRegionAddress )
{
    auto address = RifEclipseSummaryAddress::regionAddress( "ROPT", 42 );
    auto result =
        RiaSummaryAddressCollectionTools::objectIdentifierForAddress( address, RimSummaryAddressCollection::CollectionContentType::REGION );
    EXPECT_EQ( "42", result );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaSummaryAddressCollectionTools, ObjectIdentifierForWellSegmentAddress )
{
    auto address = RifEclipseSummaryAddress::wellSegmentAddress( "SOFR", "WELL-1", 7 );
    auto result =
        RiaSummaryAddressCollectionTools::objectIdentifierForAddress( address,
                                                                      RimSummaryAddressCollection::CollectionContentType::WELL_SEGMENT );
    EXPECT_EQ( "7", result );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaSummaryAddressCollectionTools, ObjectIdentifierMismatchReturnsEmpty )
{
    auto address = RifEclipseSummaryAddress::wellAddress( "WOPT", "WELL-1" );
    auto result =
        RiaSummaryAddressCollectionTools::objectIdentifierForAddress( address, RimSummaryAddressCollection::CollectionContentType::GROUP );
    EXPECT_TRUE( result.empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaSummaryAddressCollectionTools, ObjectIdentifierFieldAddressReturnsEmpty )
{
    auto address = RifEclipseSummaryAddress::fieldAddress( "FOPT" );
    auto result =
        RiaSummaryAddressCollectionTools::objectIdentifierForAddress( address, RimSummaryAddressCollection::CollectionContentType::FIELD );
    EXPECT_TRUE( result.empty() );
}

//--------------------------------------------------------------------------------------------------
// buildCurveDefs tests
//--------------------------------------------------------------------------------------------------
TEST( RiaSummaryAddressCollectionTools, BuildCurveDefsWithSingleSourceDef )
{
    auto wellAddress = RifEclipseSummaryAddress::wellAddress( "WOPT", "WELL-A" );

    RiaSummaryCurveDefinition              curveDef( nullptr, wellAddress, false );
    std::vector<RiaSummaryCurveDefinition> sourceDefs = { curveDef };

    auto result =
        RiaSummaryAddressCollectionTools::buildCurveDefs( sourceDefs, "WELL-B", RimSummaryAddressCollection::CollectionContentType::WELL, false );

    EXPECT_EQ( 1u, result.size() );
    EXPECT_EQ( "WELL-B", result[0].summaryAddressY().wellName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaSummaryAddressCollectionTools, BuildCurveDefsPreservesCasePointer )
{
    // The helper must preserve the case pointer from source curve defs. The caller
    // (handleAddressCollectionDrop) is responsible for setting the correct case on
    // source defs before calling this helper.
    RimMockSummaryCase dummyCase;

    auto wellAddress = RifEclipseSummaryAddress::wellAddress( "WOPT", "WELL-A" );

    RiaSummaryCurveDefinition              curveDef( &dummyCase, wellAddress, false );
    std::vector<RiaSummaryCurveDefinition> sourceDefs = { curveDef };

    auto result =
        RiaSummaryAddressCollectionTools::buildCurveDefs( sourceDefs, "WELL-B", RimSummaryAddressCollection::CollectionContentType::WELL, false );

    EXPECT_EQ( 1u, result.size() );
    EXPECT_EQ( &dummyCase, result[0].summaryCaseY() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaSummaryAddressCollectionTools, BuildCurveDefsWithHistoryVectors )
{
    auto wellAddress = RifEclipseSummaryAddress::wellAddress( "WOPT", "WELL-A" );

    RiaSummaryCurveDefinition              curveDef( nullptr, wellAddress, false );
    std::vector<RiaSummaryCurveDefinition> sourceDefs = { curveDef };

    auto result =
        RiaSummaryAddressCollectionTools::buildCurveDefs( sourceDefs, "WELL-B", RimSummaryAddressCollection::CollectionContentType::WELL, true );

    EXPECT_EQ( 2u, result.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaSummaryAddressCollectionTools, BuildCurveDefsWithEmptySourceDefs )
{
    std::vector<RiaSummaryCurveDefinition> sourceDefs;

    auto result =
        RiaSummaryAddressCollectionTools::buildCurveDefs( sourceDefs, "WELL-B", RimSummaryAddressCollection::CollectionContentType::WELL, false );

    EXPECT_TRUE( result.empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaSummaryAddressCollectionTools, BuildCurveDefsWithMultipleSourceDefs )
{
    auto wellAddress1 = RifEclipseSummaryAddress::wellAddress( "WOPT", "WELL-A" );
    auto wellAddress2 = RifEclipseSummaryAddress::wellAddress( "WWCT", "WELL-A" );

    RiaSummaryCurveDefinition              curveDef1( nullptr, wellAddress1, false );
    RiaSummaryCurveDefinition              curveDef2( nullptr, wellAddress2, false );
    std::vector<RiaSummaryCurveDefinition> sourceDefs = { curveDef1, curveDef2 };

    auto result =
        RiaSummaryAddressCollectionTools::buildCurveDefs( sourceDefs, "WELL-B", RimSummaryAddressCollection::CollectionContentType::WELL, false );

    EXPECT_EQ( 2u, result.size() );
}

//--------------------------------------------------------------------------------------------------
// removeExistingCurveDefs tests
//--------------------------------------------------------------------------------------------------
TEST( RiaSummaryAddressCollectionTools, RemoveExistingCurveDefsKeepsDifferentCase )
{
    RimMockSummaryCase caseA;
    RimMockSummaryCase caseB;

    auto addr = RifEclipseSummaryAddress::wellAddress( "WOPT", "WELL-B" );

    // Candidate uses caseB
    std::vector<RiaSummaryCurveDefinition> candidates = { RiaSummaryCurveDefinition( &caseB, addr, false ) };

    // Plot already has this address from caseA
    std::map<RifEclipseSummaryAddress, std::set<RimSummaryCase*>> existingSummary;
    existingSummary[addr].insert( &caseA );

    std::map<RifEclipseSummaryAddress, std::set<RimSummaryEnsemble*>> existingEnsemble;

    auto result = RiaSummaryAddressCollectionTools::removeExistingCurveDefs( candidates, existingSummary, existingEnsemble );

    // Different case, should NOT be filtered out
    EXPECT_EQ( 1u, result.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaSummaryAddressCollectionTools, RemoveExistingCurveDefsFiltersSameCase )
{
    RimMockSummaryCase caseA;

    auto addr = RifEclipseSummaryAddress::wellAddress( "WOPT", "WELL-B" );

    // Candidate uses caseA
    std::vector<RiaSummaryCurveDefinition> candidates = { RiaSummaryCurveDefinition( &caseA, addr, false ) };

    // Plot already has this exact case+address
    std::map<RifEclipseSummaryAddress, std::set<RimSummaryCase*>> existingSummary;
    existingSummary[addr].insert( &caseA );

    std::map<RifEclipseSummaryAddress, std::set<RimSummaryEnsemble*>> existingEnsemble;

    auto result = RiaSummaryAddressCollectionTools::removeExistingCurveDefs( candidates, existingSummary, existingEnsemble );

    // Same case+address, should be filtered out
    EXPECT_TRUE( result.empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaSummaryAddressCollectionTools, RemoveExistingCurveDefsPassesThroughEmpty )
{
    std::vector<RiaSummaryCurveDefinition>                            candidates;
    std::map<RifEclipseSummaryAddress, std::set<RimSummaryCase*>>     existingSummary;
    std::map<RifEclipseSummaryAddress, std::set<RimSummaryEnsemble*>> existingEnsemble;

    auto result = RiaSummaryAddressCollectionTools::removeExistingCurveDefs( candidates, existingSummary, existingEnsemble );

    EXPECT_TRUE( result.empty() );
}
