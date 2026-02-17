#include "gtest/gtest.h"

#include "RimMockSummaryCase.h"

#include "RifEclipseSummaryAddress.h"
#include "RigCaseRealizationParameters.h"

//--------------------------------------------------------------------------------------------------
// RimSummaryCase interface tests
//--------------------------------------------------------------------------------------------------
TEST( RimMockSummaryCase, DefaultCaseName )
{
    RimMockSummaryCase mockCase;
    EXPECT_EQ( "MockCase", mockCase.caseName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimMockSummaryCase, SetCaseName )
{
    RimMockSummaryCase mockCase;
    mockCase.setName( "MyCase" );
    EXPECT_EQ( "MyCase", mockCase.caseName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimMockSummaryCase, SummaryReaderReturnsSelf )
{
    RimMockSummaryCase mockCase;
    EXPECT_EQ( static_cast<RifSummaryReaderInterface*>( &mockCase ), mockCase.summaryReader() );
}

//--------------------------------------------------------------------------------------------------
// RifSummaryReaderInterface tests
//--------------------------------------------------------------------------------------------------
TEST( RimMockSummaryCase, ValuesForKnownAddress )
{
    RimMockSummaryCase mockCase;

    auto addr = RifEclipseSummaryAddress::fieldAddress( "FOPT" );
    mockCase.addVector( addr, { 0, 100, 200 }, { 1.0, 2.0, 3.0 } );

    auto [ok, vals] = mockCase.values( addr );
    EXPECT_TRUE( ok );
    ASSERT_EQ( 3u, vals.size() );
    EXPECT_DOUBLE_EQ( 1.0, vals[0] );
    EXPECT_DOUBLE_EQ( 2.0, vals[1] );
    EXPECT_DOUBLE_EQ( 3.0, vals[2] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimMockSummaryCase, ValuesForUnknownAddressReturnsFalse )
{
    RimMockSummaryCase mockCase;

    auto addr = RifEclipseSummaryAddress::fieldAddress( "FOPT" );

    auto [ok, vals] = mockCase.values( addr );
    EXPECT_FALSE( ok );
    EXPECT_TRUE( vals.empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimMockSummaryCase, TimeStepsForKnownAddress )
{
    RimMockSummaryCase mockCase;

    auto addr = RifEclipseSummaryAddress::wellAddress( "WOPT", "W-1" );
    mockCase.addVector( addr, { 10, 20, 30 }, { 5.0, 6.0, 7.0 } );

    auto ts = mockCase.timeSteps( addr );
    ASSERT_EQ( 3u, ts.size() );
    EXPECT_EQ( 10, ts[0] );
    EXPECT_EQ( 20, ts[1] );
    EXPECT_EQ( 30, ts[2] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimMockSummaryCase, TimeStepsForUnknownAddressReturnsEmpty )
{
    RimMockSummaryCase mockCase;

    auto addr = RifEclipseSummaryAddress::fieldAddress( "FOPT" );
    EXPECT_TRUE( mockCase.timeSteps( addr ).empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimMockSummaryCase, UnitNameForKnownAddress )
{
    RimMockSummaryCase mockCase;

    auto addr = RifEclipseSummaryAddress::fieldAddress( "FOPT" );
    mockCase.addVector( addr, { 0 }, { 1.0 }, "SM3" );

    EXPECT_EQ( "SM3", mockCase.unitName( addr ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimMockSummaryCase, UnitNameDefaultsToEmpty )
{
    RimMockSummaryCase mockCase;

    auto addr = RifEclipseSummaryAddress::fieldAddress( "FOPT" );
    mockCase.addVector( addr, { 0 }, { 1.0 } );

    EXPECT_TRUE( mockCase.unitName( addr ).empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimMockSummaryCase, UnitSystemIsMetric )
{
    RimMockSummaryCase mockCase;
    EXPECT_EQ( RiaDefines::EclipseUnitSystem::UNITS_METRIC, mockCase.unitSystem() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimMockSummaryCase, AllResultAddressesPopulatedByAddVector )
{
    RimMockSummaryCase mockCase;

    auto addr1 = RifEclipseSummaryAddress::fieldAddress( "FOPT" );
    auto addr2 = RifEclipseSummaryAddress::wellAddress( "WOPT", "W-1" );

    mockCase.addVector( addr1, { 0 }, { 1.0 } );
    mockCase.addVector( addr2, { 0 }, { 2.0 } );

    auto reader = mockCase.summaryReader();
    EXPECT_EQ( 2u, reader->allResultAddresses().size() );
    EXPECT_TRUE( reader->hasAddress( addr1 ) );
    EXPECT_TRUE( reader->hasAddress( addr2 ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimMockSummaryCase, KeywordCountMatchesVectorCount )
{
    RimMockSummaryCase mockCase;
    EXPECT_EQ( 0u, mockCase.keywordCount() );

    mockCase.addVector( RifEclipseSummaryAddress::fieldAddress( "FOPT" ), { 0 }, { 1.0 } );
    EXPECT_EQ( 1u, mockCase.keywordCount() );

    mockCase.addVector( RifEclipseSummaryAddress::fieldAddress( "FGPT" ), { 0 }, { 2.0 } );
    EXPECT_EQ( 2u, mockCase.keywordCount() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimMockSummaryCase, AddVectorOverwritesExistingAddress )
{
    RimMockSummaryCase mockCase;

    auto addr = RifEclipseSummaryAddress::fieldAddress( "FOPT" );
    mockCase.addVector( addr, { 0 }, { 1.0 } );
    mockCase.addVector( addr, { 0, 100 }, { 10.0, 20.0 } );

    auto [ok, vals] = mockCase.values( addr );
    EXPECT_TRUE( ok );
    ASSERT_EQ( 2u, vals.size() );
    EXPECT_DOUBLE_EQ( 10.0, vals[0] );
    EXPECT_DOUBLE_EQ( 20.0, vals[1] );

    EXPECT_EQ( 1u, mockCase.keywordCount() );
}

//--------------------------------------------------------------------------------------------------
// RimSummaryCase base class tests
//--------------------------------------------------------------------------------------------------
TEST( RimMockSummaryCase, CaseIdSetAndGet )
{
    RimMockSummaryCase mockCase;
    mockCase.setCaseId( 42 );
    EXPECT_EQ( 42, mockCase.caseId() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimMockSummaryCase, SummaryHeaderFilename )
{
    RimMockSummaryCase mockCase;
    mockCase.setSummaryHeaderFileName( "/tmp/test.SMSPEC" );
    EXPECT_EQ( "/tmp/test.SMSPEC", mockCase.summaryHeaderFilename() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimMockSummaryCase, IsObservedDataDefaultFalse )
{
    RimMockSummaryCase mockCase;
    EXPECT_FALSE( mockCase.isObservedData() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimMockSummaryCase, NativeCaseNameReturnsCaseName )
{
    RimMockSummaryCase mockCase;
    mockCase.setName( "TestCase" );
    EXPECT_EQ( "TestCase", mockCase.nativeCaseName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimMockSummaryCase, CustomCaseNameSetsDisplayName )
{
    RimMockSummaryCase mockCase;
    mockCase.setCustomCaseName( "CustomName" );
    EXPECT_EQ( "CustomName", mockCase.displayCaseName() );
    EXPECT_EQ( RimCaseDisplayNameTools::DisplayName::CUSTOM, mockCase.displayNameType() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimMockSummaryCase, DisplayNameOption )
{
    RimMockSummaryCase mockCase;
    mockCase.setDisplayNameOption( RimCaseDisplayNameTools::DisplayName::FULL_CASE_NAME );
    EXPECT_EQ( RimCaseDisplayNameTools::DisplayName::FULL_CASE_NAME, mockCase.displayNameType() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimMockSummaryCase, UnitsSystemDelegatesToReader )
{
    RimMockSummaryCase mockCase;
    EXPECT_EQ( RiaDefines::EclipseUnitSystem::UNITS_METRIC, mockCase.unitsSystem() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimMockSummaryCase, CaseRealizationParametersDefault )
{
    RimMockSummaryCase mockCase;
    EXPECT_FALSE( mockCase.hasCaseRealizationParameters() );
    EXPECT_EQ( nullptr, mockCase.caseRealizationParameters() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimMockSummaryCase, CaseRealizationParametersSetAndGet )
{
    RimMockSummaryCase mockCase;

    auto params = std::make_shared<RigCaseRealizationParameters>();
    mockCase.setCaseRealizationParameters( params );

    EXPECT_TRUE( mockCase.hasCaseRealizationParameters() );
    EXPECT_EQ( params, mockCase.caseRealizationParameters() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimMockSummaryCase, LessThanOperatorSortsByCaseName )
{
    RimMockSummaryCase caseA;
    RimMockSummaryCase caseB;
    caseA.setName( "Alpha" );
    caseB.setName( "Beta" );

    EXPECT_TRUE( caseA < caseB );
    EXPECT_FALSE( caseB < caseA );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimMockSummaryCase, LessThanOperatorEqualNames )
{
    RimMockSummaryCase caseA;
    RimMockSummaryCase caseB;
    caseA.setName( "Same" );
    caseB.setName( "Same" );

    EXPECT_FALSE( caseA < caseB );
    EXPECT_FALSE( caseB < caseA );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimMockSummaryCase, CopyFromCopiesHeaderFilename )
{
    RimMockSummaryCase source;
    source.setSummaryHeaderFileName( "/data/case.SMSPEC" );

    RimMockSummaryCase target;
    target.copyFrom( source );

    EXPECT_EQ( "/data/case.SMSPEC", target.summaryHeaderFilename() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimMockSummaryCase, ShowTreeNodesDefault )
{
    RimMockSummaryCase mockCase;
    EXPECT_TRUE( mockCase.showTreeNodes() );
}
