#include "gtest/gtest.h"

#include "RimEnsembleStatisticsCase.h"
#include "RimMockSummaryCase.h"

#include "RifEclipseSummaryAddress.h"

#include <limits>

//--------------------------------------------------------------------------------------------------
/// Three realizations with a zero value (well shut-in) at the second time step. With zero values
/// included, the mean at that time step is affected by the zero. With zero values excluded, the zero
/// should not contribute to the mean (or percentiles).
//--------------------------------------------------------------------------------------------------
TEST( RimEnsembleStatisticsCase, IncludeZeroValuesAffectsMean )
{
    auto addr = RifEclipseSummaryAddress::wellAddress( "WWCT", "W-1" );

    RimMockSummaryCase caseA;
    RimMockSummaryCase caseB;
    RimMockSummaryCase caseC;

    std::vector<time_t> timeSteps = { 0, 100, 200 };

    caseA.addVector( addr, timeSteps, { 0.1, 0.0, 0.3 } );
    caseB.addVector( addr, timeSteps, { 0.2, 0.0, 0.4 } );
    caseC.addVector( addr, timeSteps, { 0.3, 0.6, 0.5 } );

    std::vector<RimSummaryCase*> summaryCases = { &caseA, &caseB, &caseC };

    auto meanAddress = addr;
    meanAddress.setStatisticsType( RifEclipseSummaryAddressDefines::StatisticsType::MEAN );

    {
        RimEnsembleStatisticsCase statisticsCase;
        bool                      includeIncompleteCurves = true;
        bool                      includeZeroValues       = true;
        statisticsCase.calculate( summaryCases, addr, includeIncompleteCurves, { 10, 50, 90 }, includeZeroValues );

        auto [ok, meanValues] = statisticsCase.values( meanAddress );
        ASSERT_TRUE( ok );
        ASSERT_EQ( 3u, meanValues.size() );

        // Mean of 0.0, 0.0, 0.6 at the second time step
        EXPECT_NEAR( 0.2, meanValues[1], 1.0e-6 );
    }

    {
        RimEnsembleStatisticsCase statisticsCase;
        bool                      includeIncompleteCurves = true;
        bool                      includeZeroValues       = false;
        statisticsCase.calculate( summaryCases, addr, includeIncompleteCurves, { 10, 50, 90 }, includeZeroValues );

        auto [ok, meanValues] = statisticsCase.values( meanAddress );
        ASSERT_TRUE( ok );
        ASSERT_EQ( 3u, meanValues.size() );

        // Mean of only 0.6 at the second time step, as the two zero values are excluded
        EXPECT_NEAR( 0.6, meanValues[1], 1.0e-6 );
    }
}

//--------------------------------------------------------------------------------------------------
/// If every realization has a zero value at a given time step and zero values are excluded, no data
/// remains for the statistics calculation at that time step.
//--------------------------------------------------------------------------------------------------
TEST( RimEnsembleStatisticsCase, IncludeZeroValuesFalseWithAllZerosGivesInvalidMean )
{
    auto addr = RifEclipseSummaryAddress::wellAddress( "WWCT", "W-1" );

    RimMockSummaryCase caseA;
    RimMockSummaryCase caseB;

    std::vector<time_t> timeSteps = { 0, 100 };

    caseA.addVector( addr, timeSteps, { 0.1, 0.0 } );
    caseB.addVector( addr, timeSteps, { 0.2, 0.0 } );

    std::vector<RimSummaryCase*> summaryCases = { &caseA, &caseB };

    RimEnsembleStatisticsCase statisticsCase;
    bool                      includeIncompleteCurves = true;
    bool                      includeZeroValues       = false;
    statisticsCase.calculate( summaryCases, addr, includeIncompleteCurves, { 10, 50, 90 }, includeZeroValues );

    auto meanAddress = addr;
    meanAddress.setStatisticsType( RifEclipseSummaryAddressDefines::StatisticsType::MEAN );

    auto [ok, meanValues] = statisticsCase.values( meanAddress );
    ASSERT_TRUE( ok );
    ASSERT_EQ( 2u, meanValues.size() );

    EXPECT_NEAR( 0.15, meanValues[0], 1.0e-6 );
    EXPECT_EQ( std::numeric_limits<double>::infinity(), meanValues[1] );
}

//--------------------------------------------------------------------------------------------------
/// Values that are close to, but not exactly, zero are not treated as zero and are not excluded. Shut-in
/// values are reported as exact 0.0 by the simulator, so an exact comparison is used rather than an
/// epsilon, to avoid excluding small-but-valid values for e.g. low-magnitude vectors such as PI.
//--------------------------------------------------------------------------------------------------
TEST( RimEnsembleStatisticsCase, IncludeZeroValuesFalseDoesNotExcludeNearZeroValues )
{
    auto addr = RifEclipseSummaryAddress::wellAddress( "WWCT", "W-1" );

    RimMockSummaryCase caseA;
    RimMockSummaryCase caseB;

    std::vector<time_t> timeSteps = { 0, 100 };

    caseA.addVector( addr, timeSteps, { 0.1, 1.0e-9 } );
    caseB.addVector( addr, timeSteps, { 0.2, -1.0e-9 } );

    std::vector<RimSummaryCase*> summaryCases = { &caseA, &caseB };

    RimEnsembleStatisticsCase statisticsCase;
    bool                      includeIncompleteCurves = true;
    bool                      includeZeroValues       = false;
    statisticsCase.calculate( summaryCases, addr, includeIncompleteCurves, { 10, 50, 90 }, includeZeroValues );

    auto meanAddress = addr;
    meanAddress.setStatisticsType( RifEclipseSummaryAddressDefines::StatisticsType::MEAN );

    auto [ok, meanValues] = statisticsCase.values( meanAddress );
    ASSERT_TRUE( ok );
    ASSERT_EQ( 2u, meanValues.size() );

    // The near-zero values are not excluded, since they are not exactly 0.0
    EXPECT_NEAR( 0.0, meanValues[1], 1.0e-9 );
}
