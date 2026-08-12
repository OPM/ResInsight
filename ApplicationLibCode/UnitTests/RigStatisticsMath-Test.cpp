

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

#include "RigStatisticsDataCache.h"
#include "RigStatisticsMath.h"

#include "QElapsedTimer"

#include <cmath>
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
    EXPECT_DOUBLE_EQ( 7292.3587591482656, p50 );
    EXPECT_DOUBLE_EQ( 96798.640494338761, p90 );
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

        // With few samples, P10 and P90 are clamped to the boundary (min/max) values
        RigStatisticsMath::calculateStatisticsCurves( values, &p10, &p50, &p90, &mean, percentileStyle );
        EXPECT_DOUBLE_EQ( 1.0, p10 );
        EXPECT_DOUBLE_EQ( 1.0, p90 );
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
/// Default constructor arguments: linear binning, out-of-range values excluded
//--------------------------------------------------------------------------------------------------
TEST( RigStatisticsMath, LinearHistogramBinning )
{
    std::vector<size_t>    histogram;
    RigHistogramCalculator histCalc( 0.0, 10.0, 5, &histogram );

    histCalc.addData( std::vector<double>{ -0.1, 0.0, 3.0, 4.9, 7.0, 9.9, 10.0, 10.1, HUGE_VAL, -HUGE_VAL, std::nan( "" ) } );

    // Uniform bins of width 2. Values outside [0, 10] and invalid numbers are discarded.
    // The maximum value is included in the last bin.
    std::vector<size_t> expected = { 1, 1, 1, 1, 2 };
    EXPECT_EQ( expected, histogram );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigStatisticsMath, LogarithmicHistogramBinning )
{
    std::vector<size_t>    histogram;
    RigHistogramCalculator histCalc( 1.0, 1000.0, 3, &histogram, RigHistogramCalculator::BinningMode::LOGARITHMIC );

    histCalc.addData( std::vector<double>{ 0.5, 1.0, 2.0, 20.0, 200.0, 999.0, 1000.0, 2000.0 } );

    // One bin per decade: [1, 10), [10, 100), [100, 1000]. Values outside the range are discarded.
    std::vector<size_t> expected = { 2, 1, 3 };
    EXPECT_EQ( expected, histogram );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigStatisticsMath, CustomRangeExcludesOutOfRangeValues )
{
    std::vector<size_t>    histogram;
    RigHistogramCalculator histCalc( 10.0, 20.0, 2, &histogram );

    histCalc.addData( std::vector<double>{ 5.0, 12.0, 18.0, 20.0, 25.0 } );

    std::vector<size_t> expected = { 1, 2 };
    EXPECT_EQ( expected, histogram );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigStatisticsMath, CustomRangeClampsToBoundaryBins )
{
    {
        std::vector<size_t>    histogram;
        RigHistogramCalculator histCalc( 10.0,
                                         20.0,
                                         2,
                                         &histogram,
                                         RigHistogramCalculator::BinningMode::LINEAR,
                                         RigHistogramCalculator::OutOfRangeHandling::INCLUDE_IN_BOUNDARY_BINS );

        histCalc.addData( std::vector<double>{ 5.0, 12.0, 18.0, 20.0, 25.0, HUGE_VAL, -HUGE_VAL, std::nan( "" ) } );

        // Below range -> first bin, above range -> last bin, invalid numbers still skipped
        std::vector<size_t> expected = { 2, 3 };
        EXPECT_EQ( expected, histogram );
    }

    {
        std::vector<size_t>    histogram;
        RigHistogramCalculator histCalc( 10.0,
                                         1000.0,
                                         2,
                                         &histogram,
                                         RigHistogramCalculator::BinningMode::LOGARITHMIC,
                                         RigHistogramCalculator::OutOfRangeHandling::INCLUDE_IN_BOUNDARY_BINS );

        histCalc.addData( std::vector<double>{ 5.0, 50.0, 2000.0 } );

        std::vector<size_t> expected = { 2, 1 };
        EXPECT_EQ( expected, histogram );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigStatisticsMath, LogarithmicBinningNonPositiveValues )
{
    {
        std::vector<size_t>    histogram;
        RigHistogramCalculator histCalc( 1.0, 100.0, 2, &histogram, RigHistogramCalculator::BinningMode::LOGARITHMIC );

        histCalc.addData( std::vector<double>{ 0.0, -5.0, 5.0 } );

        std::vector<size_t> expected = { 1, 0 };
        EXPECT_EQ( expected, histogram );
    }

    {
        std::vector<size_t>    histogram;
        RigHistogramCalculator histCalc( 1.0,
                                         100.0,
                                         2,
                                         &histogram,
                                         RigHistogramCalculator::BinningMode::LOGARITHMIC,
                                         RigHistogramCalculator::OutOfRangeHandling::INCLUDE_IN_BOUNDARY_BINS );

        histCalc.addData( std::vector<double>{ 0.0, -5.0, 5.0 } );

        // Non-positive values cannot be represented on a logarithmic scale and count in the first bin
        std::vector<size_t> expected = { 3, 0 };
        EXPECT_EQ( expected, histogram );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigStatisticsMath, LogarithmicHistogramPercentiles )
{
    std::vector<size_t>    histogram;
    RigHistogramCalculator histCalc( 1.0, 10000.0, 4, &histogram, RigHistogramCalculator::BinningMode::LOGARITHMIC );

    histCalc.addData( std::vector<double>{ 1.0, 10.0, 100.0, 1000.0, 10000.0 } );

    // Percentile interpolation happens in log10 space, results are transformed back to the value domain
    double p10 = histCalc.calculatePercentil( 0.1, RigStatisticsMath::PercentileStyle::REGULAR );
    double p50 = histCalc.calculatePercentil( 0.5, RigStatisticsMath::PercentileStyle::REGULAR );

    EXPECT_DOUBLE_EQ( std::pow( 10.0, 0.5 ), p10 );
    EXPECT_DOUBLE_EQ( std::pow( 10.0, 2.5 ), p50 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigStatisticsMath, HistogramDegenerateRanges )
{
    {
        // Zero range collapses to a single bin
        std::vector<size_t>    histogram;
        RigHistogramCalculator histCalc( 5.0, 5.0, 3, &histogram );
        histCalc.addValue( 5.0 );

        std::vector<size_t> expected = { 1 };
        EXPECT_EQ( expected, histogram );
    }

    {
        // Logarithmic binning with a non-positive minimum falls back to linear binning
        std::vector<size_t>    histogram;
        RigHistogramCalculator histCalc( 0.0, 10.0, 2, &histogram, RigHistogramCalculator::BinningMode::LOGARITHMIC );
        histCalc.addValue( 7.0 );

        std::vector<size_t> expected = { 0, 1 };
        EXPECT_EQ( expected, histogram );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
namespace
{
class VectorBackedStatisticsCalculator : public RigStatisticsCalculator
{
public:
    explicit VectorBackedStatisticsCalculator( const std::vector<std::vector<double>>& valuesPerTimeStep )
        : m_valuesPerTimeStep( valuesPerTimeStep )
    {
    }

    void minMaxCellScalarValues( size_t timeStepIndex, double& min, double& max ) override
    {
        MinMaxAccumulator accumulator( min, max );
        accumulator.addData( m_valuesPerTimeStep[timeStepIndex] );
        min = accumulator.min;
        max = accumulator.max;
    }

    void posNegClosestToZero( size_t timeStepIndex, double& pos, double& neg ) override
    {
        PosNegAccumulator accumulator( pos, neg );
        accumulator.addData( m_valuesPerTimeStep[timeStepIndex] );
        pos = accumulator.pos;
        neg = accumulator.neg;
    }

    void valueSumAndSampleCount( size_t timeStepIndex, double& valueSum, size_t& sampleCount ) override
    {
        SumCountAccumulator accumulator( valueSum, sampleCount );
        accumulator.addData( m_valuesPerTimeStep[timeStepIndex] );
        valueSum    = accumulator.valueSum;
        sampleCount = accumulator.sampleCount;
    }

    void addDataToHistogramCalculator( size_t timeStepIndex, RigHistogramCalculator& histogramCalculator ) override
    {
        histogramCalculator.addData( m_valuesPerTimeStep[timeStepIndex] );
    }

    void uniqueValues( size_t timeStepIndex, std::set<int>& values ) override {}

    size_t timeStepCount() override { return m_valuesPerTimeStep.size(); }

private:
    std::vector<std::vector<double>> m_valuesPerTimeStep;
};
} // namespace

//--------------------------------------------------------------------------------------------------
/// computeHistogram() fills a custom-configured histogram calculator without disturbing the
/// statistics cached for other consumers
//--------------------------------------------------------------------------------------------------
TEST( RigStatisticsMath, StatisticsDataCacheComputeHistogramPassThrough )
{
    cvf::ref<VectorBackedStatisticsCalculator> calculator = new VectorBackedStatisticsCalculator( { { 1.0, 10.0, 100.0 }, { 1000.0, 0.5 } } );

    cvf::ref<RigStatisticsDataCache> cache = new RigStatisticsDataCache( calculator.p() );

    std::vector<size_t> cachedHistogramBefore = cache->cellScalarValuesHistogram();

    double minBefore, maxBefore, p10Before, p90Before;
    cache->minMaxCellScalarValues( minBefore, maxBefore );
    cache->p10p90CellScalarValues( p10Before, p90Before );

    {
        std::vector<size_t>    histogram;
        RigHistogramCalculator histCalc( 1.0, 1000.0, 3, &histogram, RigHistogramCalculator::BinningMode::LOGARITHMIC );
        cache->computeHistogram( histCalc );

        // All time steps: {1, 10, 100, 1000, 0.5} in decade bins, 0.5 is below range and discarded
        std::vector<size_t> expected = { 1, 1, 2 };
        EXPECT_EQ( expected, histogram );
    }

    {
        std::vector<size_t>    histogram;
        RigHistogramCalculator histCalc( 1.0, 1000.0, 3, &histogram, RigHistogramCalculator::BinningMode::LOGARITHMIC );
        cache->computeHistogram( 0, histCalc );

        std::vector<size_t> expected = { 1, 1, 1 };
        EXPECT_EQ( expected, histogram );
    }

    // The cached statistics are not perturbed by the custom histogram computations
    EXPECT_EQ( cachedHistogramBefore, cache->cellScalarValuesHistogram() );

    double minAfter, maxAfter, p10After, p90After;
    cache->minMaxCellScalarValues( minAfter, maxAfter );
    cache->p10p90CellScalarValues( p10After, p90After );

    EXPECT_DOUBLE_EQ( minBefore, minAfter );
    EXPECT_DOUBLE_EQ( maxBefore, maxAfter );
    EXPECT_DOUBLE_EQ( p10Before, p10After );
    EXPECT_DOUBLE_EQ( p90Before, p90After );
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
