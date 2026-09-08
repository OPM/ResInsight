#include "gtest/gtest.h"

#include "RifEclipseSummaryAddress.h"
#include "RigCaseRealizationParameters.h"
#include "RigEnsembleParameter.h"
#include "RimMockSummaryCase.h"
#include "RimSummaryEnsemble.h"

#include <cmath>
#include <memory>

namespace
{
//--------------------------------------------------------------------------------------------------
/// Creates a mock case with a numeric realization parameter and a single-valued FOPT vector at time 0.
//--------------------------------------------------------------------------------------------------
RimSummaryCase* createCaseWithParameterAndValue( int realizationNumber, double parameterValue, double resultValue )
{
    auto* mockCase = static_cast<RimMockSummaryCase*>( createMockCase( realizationNumber ) );

    auto parameters = mockCase->caseRealizationParameters();
    parameters->addParameter( "PARAM1", parameterValue );

    // Note: the sentinel "no closest timestep found yet" value used internally by
    // RimSummaryEnsemble::parameterCorrelations() is time_t( 0 ), so the query timestep used in the
    // tests below must be non-zero for the closest-timestep search to pick up the value at all.
    mockCase->addVector( RifEclipseSummaryAddress::fieldAddress( "FOPT" ), { 100 }, { resultValue } );

    return mockCase;
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// A realization with a NaN/Inf value in the calculated vector must be excluded from the correlation
/// computation instead of turning the whole result into NaN.
//--------------------------------------------------------------------------------------------------
TEST( RimSummaryEnsemble, ParameterCorrelations_NanRealizationExcluded )
{
    RimSummaryEnsemble ensemble;
    ensemble.setAsEnsemble( true );

    // Perfectly correlated realizations
    ensemble.addCase( createCaseWithParameterAndValue( 0, 1.0, 10.0 ), false );
    ensemble.addCase( createCaseWithParameterAndValue( 1, 2.0, 20.0 ), false );

    // A realization with an invalid (NaN) result value, e.g. from a calculated vector dividing by zero
    ensemble.addCase( createCaseWithParameterAndValue( 2, 3.0, std::numeric_limits<double>::quiet_NaN() ), false );

    auto address = RifEclipseSummaryAddress::fieldAddress( "FOPT" );

    auto correlations = ensemble.parameterCorrelations( address, 100 );
    ASSERT_EQ( size_t( 1 ), correlations.size() );

    double correlation = correlations.front().second;
    EXPECT_TRUE( std::isfinite( correlation ) );
    EXPECT_NEAR( 1.0, correlation, 1.0e-6 );
}

//--------------------------------------------------------------------------------------------------
/// A realization with an infinite value in the calculated vector must be excluded from the
/// correlation computation instead of turning the whole result into NaN/Inf.
//--------------------------------------------------------------------------------------------------
TEST( RimSummaryEnsemble, ParameterCorrelations_InfRealizationExcluded )
{
    RimSummaryEnsemble ensemble;
    ensemble.setAsEnsemble( true );

    ensemble.addCase( createCaseWithParameterAndValue( 0, 1.0, 10.0 ), false );
    ensemble.addCase( createCaseWithParameterAndValue( 1, 2.0, 20.0 ), false );
    ensemble.addCase( createCaseWithParameterAndValue( 2, 3.0, std::numeric_limits<double>::infinity() ), false );

    auto address = RifEclipseSummaryAddress::fieldAddress( "FOPT" );

    auto correlations = ensemble.parameterCorrelations( address, 100 );
    ASSERT_EQ( size_t( 1 ), correlations.size() );

    double correlation = correlations.front().second;
    EXPECT_TRUE( std::isfinite( correlation ) );
    EXPECT_NEAR( 1.0, correlation, 1.0e-6 );
}

//--------------------------------------------------------------------------------------------------
/// Reproduces a NaN correlation value reaching the tornado plot without any NaN/Inf value in the
/// input data: extremely large but finite parameter/result magnitudes overflow the sum-of-squares in
/// RigStatisticsTools::pearsonCorrelation() to +Inf, and the resulting Inf/Inf division yields NaN.
/// RimSummaryEnsemble::parameterCorrelations() must guard against surfacing that NaN to the plot.
//--------------------------------------------------------------------------------------------------
TEST( RimSummaryEnsemble, ParameterCorrelations_OverflowNanGuarded )
{
    RimSummaryEnsemble ensemble;
    ensemble.setAsEnsemble( true );

    // Every value here is finite (std::isfinite is true), but the magnitudes are large enough that
    // pearsonCorrelation() overflows internally and returns NaN. See RigStatisticsTools-Test.cpp,
    // OverflowProducesNan, for a direct reproduction of that overflow.
    ensemble.addCase( createCaseWithParameterAndValue( 0, 1.0e200, 1.0e200 ), false );
    ensemble.addCase( createCaseWithParameterAndValue( 1, 2.0e200, 2.0e200 ), false );
    ensemble.addCase( createCaseWithParameterAndValue( 2, 3.0e200, 3.0e200 ), false );

    auto address = RifEclipseSummaryAddress::fieldAddress( "FOPT" );

    auto correlations = ensemble.parameterCorrelations( address, 100 );
    ASSERT_EQ( size_t( 1 ), correlations.size() );

    double correlation = correlations.front().second;
    EXPECT_TRUE( std::isfinite( correlation ) ) << "Tornado plot must never receive a NaN/Inf correlation value";
    EXPECT_FALSE( std::isnan( correlation ) );
}
