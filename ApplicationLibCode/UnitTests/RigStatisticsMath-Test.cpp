

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RigStatisticsMath.h"

#include "QElapsedTimer"

#include <numeric>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigStatisticsMath, BasicTest )
{
    std::vector<double> values;
    values.push_back( HUGE_VAL );
    values.push_back( 2788.2723335651900 );
    values.push_back( -22481.0927881701000 );
    values.push_back( 68778.6851686236000 );
    values.push_back( -76092.8157632591000 );
    values.push_back( 6391.97999909729003 );
    values.push_back( 65930.1200169780000 );
    values.push_back( -27696.2320267235000 );
    values.push_back( HUGE_VAL );
    values.push_back( HUGE_VAL );
    values.push_back( 96161.7546348456000 );
    values.push_back( 73875.6716288563000 );
    values.push_back( 80720.4378655615000 );
    values.push_back( -98649.8109937874000 );
    values.push_back( 99372.9362079615000 );
    values.push_back( -HUGE_VAL );
    values.push_back( -57020.4389966513000 );

    double min, max, sum, range, mean, stdev;
    RigStatisticsMath::calculateBasicStatistics( values, &min, &max, &sum, &range, &mean, &stdev );

    EXPECT_DOUBLE_EQ( -98649.8109937874000, min );
    EXPECT_DOUBLE_EQ( 99372.9362079615000, max );
    EXPECT_DOUBLE_EQ( 212079.46728689762, sum );
    EXPECT_DOUBLE_EQ( 198022.7472017490000, range );
    EXPECT_DOUBLE_EQ( 16313.8051759152000, mean );
    EXPECT_DOUBLE_EQ( 66104.391542887200, stdev );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigStatisticsMath, RankPercentiles )
{
    std::vector<double> values;
    values.push_back( -HUGE_VAL );
    values.push_back( 2788.2723335651900 );
    values.push_back( -22481.0927881701000 );
    values.push_back( 68778.6851686236000 );
    values.push_back( -76092.8157632591000 );
    values.push_back( 6391.97999909729003 );
    values.push_back( 65930.1200169780000 );
    values.push_back( -27696.2320267235000 );
    values.push_back( HUGE_VAL );
    values.push_back( HUGE_VAL );
    values.push_back( 96161.7546348456000 );
    values.push_back( 73875.6716288563000 );
    values.push_back( 80720.4378655615000 );
    values.push_back( -98649.8109937874000 );
    values.push_back( 99372.9362079615000 );
    values.push_back( HUGE_VAL );
    values.push_back( -57020.4389966513000 );

    std::vector<double> resultValues;
    resultValues.push_back( 10 );
    resultValues.push_back( 40 );
    resultValues.push_back( 50 );
    resultValues.push_back( 90 );
    auto pVals = RigStatisticsMath::calculateNearestRankPercentiles( values, resultValues, RigStatisticsMath::PercentileStyle::REGULAR );

    ASSERT_TRUE( pVals.has_value() );
    EXPECT_DOUBLE_EQ( -76092.8157632591000, ( *pVals )[0] );
    EXPECT_DOUBLE_EQ( 2788.2723335651900, ( *pVals )[1] );
    EXPECT_DOUBLE_EQ( 6391.979999097290, ( *pVals )[2] );
    EXPECT_DOUBLE_EQ( 96161.7546348456000, ( *pVals )[3] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigStatisticsMath, HistogramPercentiles )
{
    std::vector<double> values;
    values.push_back( HUGE_VAL );
    values.push_back( 2788.2723335651900 );
    values.push_back( -22481.0927881701000 );
    values.push_back( 68778.6851686236000 );
    values.push_back( -76092.8157632591000 );
    values.push_back( 6391.97999909729003 );
    values.push_back( 65930.1200169780000 );
    values.push_back( -27696.2320267235000 );
    values.push_back( -HUGE_VAL );
    values.push_back( -HUGE_VAL );
    values.push_back( 96161.7546348456000 );
    values.push_back( 73875.6716288563000 );
    values.push_back( 80720.4378655615000 );
    values.push_back( -98649.8109937874000 );
    values.push_back( 99372.9362079615000 );
    values.push_back( HUGE_VAL );
    values.push_back( -57020.4389966513000 );

    double min, max, range, mean, stdev;
    RigStatisticsMath::calculateBasicStatistics( values, &min, &max, nullptr, &range, &mean, &stdev );

    std::vector<size_t>    histogram;
    RigHistogramCalculator histCalc( min, max, 100, &histogram );
    histCalc.addData( values );

    double p10, p50, p90;
    p10 = histCalc.calculatePercentil( 0.1, RigStatisticsMath::PercentileStyle::REGULAR );
    p50 = histCalc.calculatePercentil( 0.5, RigStatisticsMath::PercentileStyle::REGULAR );
    p90 = histCalc.calculatePercentil( 0.9, RigStatisticsMath::PercentileStyle::REGULAR );

    EXPECT_DOUBLE_EQ( -76273.240559989776, p10 );
    EXPECT_DOUBLE_EQ( 5312.1312871307755, p50 );
    EXPECT_DOUBLE_EQ( 94818.413022321271, p90 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigStatisticsMath, InterpolatedPercentiles )
{
    std::vector<double> values;
    values.push_back( HUGE_VAL );
    values.push_back( 2788.2723335651900 );
    values.push_back( -22481.0927881701000 );
    values.push_back( 68778.6851686236000 );
    values.push_back( -76092.8157632591000 );
    values.push_back( 6391.97999909729003 );
    values.push_back( 65930.1200169780000 );
    values.push_back( -27696.2320267235000 );
    values.push_back( HUGE_VAL );
    values.push_back( HUGE_VAL );
    values.push_back( 96161.7546348456000 );
    values.push_back( 73875.6716288563000 );
    values.push_back( 80720.4378655615000 );
    values.push_back( -98649.8109937874000 );
    values.push_back( 99372.9362079615000 );
    values.push_back( HUGE_VAL );
    values.push_back( -57020.4389966513000 );

    std::vector<double> resultValues;
    resultValues.push_back( 10 );
    resultValues.push_back( 40 );
    resultValues.push_back( 50 );
    resultValues.push_back( 90 );
    auto pVals = RigStatisticsMath::calculateInterpolatedPercentiles( values, resultValues, RigStatisticsMath::PercentileStyle::REGULAR );

    ASSERT_TRUE( pVals.has_value() );
    EXPECT_DOUBLE_EQ( -72278.340409937548, ( *pVals )[0] );
    EXPECT_DOUBLE_EQ( -2265.6006907818496, ( *pVals )[1] );
    EXPECT_DOUBLE_EQ( 6391.9799990972897, ( *pVals )[2] );
    EXPECT_DOUBLE_EQ( 93073.49128098879, ( *pVals )[3] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigStatisticsMath, Accumulators )
{
    std::vector<double> values;

    const double v1 = 2788.2723335651900;
    const double v2 = 68778.6851686236000;
    const double v3 = -98649.8109937874000;
    const double v4 = -57020.4389966513000;

    values.push_back( HUGE_VAL );
    values.push_back( v1 );
    values.push_back( v2 );
    values.push_back( -HUGE_VAL );
    values.push_back( v3 );
    values.push_back( HUGE_VAL );
    values.push_back( v4 );

    {
        MinMaxAccumulator acc;
        acc.addData( values );

        EXPECT_DOUBLE_EQ( v3, acc.min );
        EXPECT_DOUBLE_EQ( v2, acc.max );
    }

    {
        PosNegAccumulator acc;
        acc.addData( values );

        EXPECT_DOUBLE_EQ( v1, acc.pos );
        EXPECT_DOUBLE_EQ( v4, acc.neg );
    }

    {
        SumCountAccumulator acc;
        acc.addData( values );

        const double sum = v1 + v2 + v3 + v4;

        EXPECT_FALSE( std::isinf( acc.valueSum ) );

        EXPECT_DOUBLE_EQ( sum, acc.valueSum );
        EXPECT_EQ( 4u, acc.sampleCount );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigStatisticsMath, calculateStatisticsCurves )
{
    RigStatisticsMath::PercentileStyle percentileStyle = RigStatisticsMath::PercentileStyle::REGULAR;

    {
        std::vector<double> values;

        double mean = HUGE_VAL;
        double p10  = HUGE_VAL;
        double p50  = HUGE_VAL;
        double p90  = HUGE_VAL;

        RigStatisticsMath::calculateStatisticsCurves( values, &p10, &p50, &p90, &mean, percentileStyle );
        EXPECT_TRUE( std::isinf( p10 ) );
        EXPECT_TRUE( std::isinf( p50 ) );
        EXPECT_TRUE( std::isinf( p90 ) );
        EXPECT_TRUE( std::isinf( mean ) );
    }

    {
        std::vector<double> values{
            1.0,
            1.0,
            1.0,
        };

        double mean = HUGE_VAL;
        double p10  = HUGE_VAL;
        double p50  = HUGE_VAL;
        double p90  = HUGE_VAL;

        // If we have few samples, P10 and P90 cannot be computed
        RigStatisticsMath::calculateStatisticsCurves( values, &p10, &p50, &p90, &mean, percentileStyle );
        EXPECT_TRUE( std::isinf( p10 ) );
        EXPECT_TRUE( std::isinf( p90 ) );
        EXPECT_DOUBLE_EQ( 1.0, p50 );
        EXPECT_DOUBLE_EQ( 1.0, mean );
    }

    {
        std::vector<double> values{ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 2.0, 5.0, 10.0 };

        double mean = HUGE_VAL;
        double p10  = HUGE_VAL;
        double p50  = HUGE_VAL;
        double p90  = HUGE_VAL;

        RigStatisticsMath::calculateStatisticsCurves( values, &p10, &p50, &p90, &mean, percentileStyle );
        EXPECT_DOUBLE_EQ( 1.0, p10 );
        EXPECT_DOUBLE_EQ( 1.0, p50 );
        EXPECT_FALSE( std::isinf( p90 ) );
        EXPECT_FALSE( std::isinf( mean ) );
    }

    {
        std::vector<double> values{ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };

        double mean = HUGE_VAL;
        double p10  = HUGE_VAL;
        double p50  = HUGE_VAL;
        double p90  = HUGE_VAL;

        RigStatisticsMath::calculateStatisticsCurves( values, &p10, &p50, &p90, &mean, percentileStyle );
        EXPECT_DOUBLE_EQ( 1.0, p10 );
        EXPECT_DOUBLE_EQ( 1.0, p50 );
        EXPECT_DOUBLE_EQ( 1.0, p90 );
        EXPECT_DOUBLE_EQ( 1.0, mean );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigStatisticsMath, calculateMean )
{
    // Empty vector should return HUGE_VAL
    {
        std::vector<double> values;
        EXPECT_DOUBLE_EQ( HUGE_VAL, RigStatisticsMath::calculateMean( values ) );
    }

    // Single value
    {
        std::vector<double> values{ 42.0 };
        EXPECT_DOUBLE_EQ( 42.0, RigStatisticsMath::calculateMean( values ) );
    }

    // Multiple valid values
    {
        std::vector<double> values{ 1.0, 2.0, 3.0, 4.0, 5.0 };
        EXPECT_DOUBLE_EQ( 3.0, RigStatisticsMath::calculateMean( values ) );
    }

    // Values with HUGE_VAL entries that should be filtered out
    {
        std::vector<double> values{ HUGE_VAL, 10.0, 20.0, -HUGE_VAL, 30.0, HUGE_VAL };
        EXPECT_DOUBLE_EQ( 20.0, RigStatisticsMath::calculateMean( values ) );
    }

    // All invalid values should return HUGE_VAL
    {
        std::vector<double> values{ HUGE_VAL, -HUGE_VAL, HUGE_VAL };
        EXPECT_DOUBLE_EQ( HUGE_VAL, RigStatisticsMath::calculateMean( values ) );
    }

    // Negative values
    {
        std::vector<double> values{ -10.0, -20.0, -30.0 };
        EXPECT_DOUBLE_EQ( -20.0, RigStatisticsMath::calculateMean( values ) );
    }

    // Mixed positive and negative values summing to zero
    {
        std::vector<double> values{ -5.0, 5.0 };
        EXPECT_DOUBLE_EQ( 0.0, RigStatisticsMath::calculateMean( values ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigStatisticsMath, DISABLED_performanceTesting )
{
    RigStatisticsMath::PercentileStyle percentileStyle = RigStatisticsMath::PercentileStyle::REGULAR;
    {
        size_t timerCount = 10;
        for ( size_t t = 0; t < timerCount; t++ )
        {
            QElapsedTimer timer;
            timer.start();

            size_t iterationCount = 10000;
            for ( size_t i = 0; i < iterationCount; i++ )
            {
                size_t              numberOfValues = 200;
                std::vector<double> values( numberOfValues );
                std::iota( values.begin(), values.end(), numberOfValues );

                double mean = HUGE_VAL;
                double p10  = HUGE_VAL;
                double p50  = HUGE_VAL;
                double p90  = HUGE_VAL;

                RigStatisticsMath::calculateStatisticsCurves( values, &p10, &p50, &p90, &mean, percentileStyle );
            }

            auto testDuration = timer.elapsed();
            std::cout << testDuration << "\n";
        }
    }
}
