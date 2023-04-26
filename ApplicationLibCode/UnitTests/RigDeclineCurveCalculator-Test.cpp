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
