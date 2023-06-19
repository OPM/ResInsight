/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "gtest/gtest.h"

#include "RiaQDateTimeTools.h"
#include "RiaTimeTTools.h"
#include "RimSummaryRegressionAnalysisCurve.h"

#include <QDateTime>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimSummaryRegressionAnalysisCurve, getOutputTimeStepsNoForecast )
{
    const std::vector<time_t> timeSteps = { 100000 };
    const std::vector<time_t> output =
        RimSummaryRegressionAnalysisCurve::getOutputTimeSteps( timeSteps, 0, 0, RimSummaryRegressionAnalysisCurve::ForecastUnit::MONTHS );

    ASSERT_EQ( timeSteps, output );
}

TEST( RimSummaryRegressionAnalysisCurve, getOutputTimeStepsForwardForecast )
{
    QDateTime                 dt        = RiaQDateTimeTools::fromYears( 2020 );
    const std::vector<time_t> timeSteps = { RiaTimeTTools::fromQDateTime( dt ) };

    int                       forecastBackward = 0;
    int                       forecastForward  = 1;
    const std::vector<time_t> output =
        RimSummaryRegressionAnalysisCurve::getOutputTimeSteps( timeSteps,
                                                               forecastBackward,
                                                               forecastForward,
                                                               RimSummaryRegressionAnalysisCurve::ForecastUnit::YEARS );

    ASSERT_EQ( output.size(), 51u );

    // First output value should be the original value in time steps
    ASSERT_EQ( timeSteps[0], output[0] );

    QDateTime oneYearLater = dt.addYears( 1 );
    for ( size_t i = 1; i < output.size(); i++ )
    {
        auto d = RiaQDateTimeTools::fromTime_t( output[i] );
        ASSERT_FALSE( RiaQDateTimeTools::lessThan( d, dt ) );
        ASSERT_TRUE( RiaQDateTimeTools::lessThan( d, oneYearLater ) );
    }
    //
}
