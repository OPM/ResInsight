/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023 Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "gtest/gtest.h"

#include "RigDeclineCurveCalculator.h"

#include <vector>

// Test case from https://web.archive.org/web/20120710104333/http://infohost.nmt.edu/~petro/faculty/Kelly/Deline.pdf
// Example Problem 8-1:
// "Given that a well has declined from 100 stb/day to 96 stb/day during a one-month period,
//  use the exponential decline model to perform the following tasks:
//  Predict the production rate after 11 more months."

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigDeclineCurveCalculatorTests, computeDeclineRateExample )
{
    double t0                    = 0.0;
    double q0                    = 100.0;
    double t1                    = 1.0;
    double q1                    = 96.0;
    double expectedDeclineRateDi = 0.04082;

    double declineRate = RigDeclineCurveCalculator::computeDeclineRate( t0, q0, t1, q1 );

    EXPECT_NEAR( expectedDeclineRateDi, declineRate, 0.005 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigDeclineCurveCalculatorTests, exponentialFlowRateExample )
{
    double initialProductionRateQi = 96.0;
    double initialDeclineRateDi    = 0.04082;
    double timeSinceStart          = 11.0;

    double q = RigDeclineCurveCalculator::computeFlowRateExponentialDecline( initialProductionRateQi, initialDeclineRateDi, timeSinceStart );

    EXPECT_NEAR( q, 61.27, 0.005 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigDeclineCurveCalculatorTests, fitRecoversParametersFromSyntheticData )
{
    const double qi = 100.0;
    const double di = 0.4 / ( 365.25 * 24.0 * 3600.0 ); // per second
    const double b  = 0.5;

    std::vector<double> times;
    std::vector<double> exponential, harmonic, hyperbolic;
    for ( int i = -20; i <= 0; i++ )
    {
        const double t = i * 30.0 * 24.0 * 3600.0;
        times.push_back( t );
        exponential.push_back( RigDeclineCurveCalculator::computeFlowRateExponentialDecline( qi, di, t ) );
        harmonic.push_back( RigDeclineCurveCalculator::computeFlowRateHarmonicDecline( qi, di, t ) );
        hyperbolic.push_back( RigDeclineCurveCalculator::computeFlowRateHyperbolicDecline( qi, di, t, b ) );
    }

    auto expFit = RigDeclineCurveCalculator::fitExponentialDecline( times, exponential );
    ASSERT_TRUE( expFit.has_value() );
    EXPECT_NEAR( qi, expFit->first, 1.0e-6 );
    EXPECT_NEAR( di, expFit->second, 1.0e-12 );

    auto harmonicFit = RigDeclineCurveCalculator::fitHarmonicDecline( times, harmonic );
    ASSERT_TRUE( harmonicFit.has_value() );
    EXPECT_NEAR( qi, harmonicFit->first, 1.0e-6 );
    EXPECT_NEAR( di, harmonicFit->second, 1.0e-12 );

    auto hyperbolicFit = RigDeclineCurveCalculator::fitHyperbolicDecline( times, hyperbolic, b );
    ASSERT_TRUE( hyperbolicFit.has_value() );
    EXPECT_NEAR( qi, hyperbolicFit->first, 1.0e-6 );
    EXPECT_NEAR( di, hyperbolicFit->second, 1.0e-12 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigDeclineCurveCalculatorTests, fitFailsWithTooFewValidPoints )
{
    EXPECT_FALSE( RigDeclineCurveCalculator::fitExponentialDecline( { -10.0, 0.0 }, { 0.0, 5.0 } ).has_value() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigDeclineCurveCalculatorTests, historyIsBackwardExtrapolationOfFittedCurve )
{
    const double day = 24.0 * 3600.0;

    using Calc = RigDeclineCurveCalculator;
    Calc::DeclineModel model{ Calc::DeclineCurveType::EXPONENTIAL, Calc::FitMethod::REGRESSION, 0.5, false };

    // Noisy history around an exponential decline
    std::vector<time_t> timeSteps = { 0, static_cast<time_t>( 100 * day ), static_cast<time_t>( 200 * day ), static_cast<time_t>( 300 * day ) };
    std::vector<double> values = { 100.0, 80.0, 66.0, 50.0 };

    auto result = Calc::computeDeclineCurveValues( model, values, timeSteps, { static_cast<time_t>( 400 * day ) } );
    ASSERT_TRUE( result.has_value() );
    ASSERT_EQ( 5u, result->size() );

    EXPECT_NE( values[0], ( *result )[0] );
    EXPECT_GT( ( *result )[0], ( *result )[1] );
    EXPECT_GT( ( *result )[3], ( *result )[4] );
}