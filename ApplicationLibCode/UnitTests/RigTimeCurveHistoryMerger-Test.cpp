#include "gtest/gtest.h"

#include "RiaCurveMerger.h"

#include <algorithm>
#include <chrono>
#include <cmath> // Needed for HUGE_VAL on Linux
#include <iostream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaTimeHistoryCurveMergerTest, TestDateInterpolation )
{
    std::vector<double> values{ 2.0, 3.5, 5.0, 6.0 };
    std::vector<time_t> timeSteps{ 1, 5, 10, 15 };

    auto interpolationMethod = RiaCurveDefines::InterpolationMethod::LINEAR;

    {
        double val = RiaTimeHistoryCurveMerger::interpolatedYValue( 1, timeSteps, values, interpolationMethod );

        EXPECT_EQ( 2.0, val );
    }

    {
        double val = RiaTimeHistoryCurveMerger::interpolatedYValue( 0, timeSteps, values, interpolationMethod );

        EXPECT_EQ( HUGE_VAL, val );
    }

    {
        double val = RiaTimeHistoryCurveMerger::interpolatedYValue( 20, timeSteps, values, interpolationMethod );

        EXPECT_EQ( HUGE_VAL, val );
    }

    {
        double val = RiaTimeHistoryCurveMerger::interpolatedYValue( 3, timeSteps, values, interpolationMethod );

        EXPECT_EQ( 2.75, val );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaTimeHistoryCurveMergerTest, InterpolationOfSortedCurveMatchesLinearSearch )
{
    std::vector<time_t> timeSteps{ 3, 4, 10, 11, 25, 40, 41, 57 };
    std::vector<double> values{ 1.0, 2.5, 3.0, -4.0, 8.0, 9.5, 0.5, 7.0 };

    for ( time_t t = -2; t < 70; t++ )
    {
        double expected = HUGE_VAL;
        for ( size_t i = 0; i < timeSteps.size(); i++ )
        {
            if ( timeSteps[i] == t )
            {
                expected = values[i];
                break;
            }
            if ( i + 1 < timeSteps.size() && timeSteps[i] < t && t < timeSteps[i + 1] )
            {
                const double w = static_cast<double>( t - timeSteps[i] ) / static_cast<double>( timeSteps[i + 1] - timeSteps[i] );
                expected       = values[i] * ( 1.0 - w ) + values[i + 1] * w;
                break;
            }
        }

        const double actual =
            RiaTimeHistoryCurveMerger::interpolatedYValue( t, timeSteps, values, RiaCurveDefines::InterpolationMethod::LINEAR );
        if ( expected == HUGE_VAL )
        {
            EXPECT_EQ( HUGE_VAL, actual ) << "t = " << t;
        }
        else
        {
            EXPECT_NEAR( expected, actual, 1.0e-12 ) << "t = " << t;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaTimeHistoryCurveMergerTest, ExtractIntervalsWithSameTimeSteps )
{
    std::vector<double> valuesA{ HUGE_VAL, 1.0, HUGE_VAL, 2.0, 2.5, 3.0, 4.0, 5.0, 6.0, HUGE_VAL };
    std::vector<double> valuesB{ 10, 20, 30, 40, 45, HUGE_VAL, HUGE_VAL, 5.0, 6.0, HUGE_VAL };

    EXPECT_EQ( valuesA.size(), valuesB.size() );

    std::vector<time_t> timeSteps;
    for ( size_t i = 0; i < 10; i++ )
    {
        timeSteps.push_back( i );
    }

    RiaTimeHistoryCurveMerger interpolate( RiaCurveDefines::InterpolationMethod::LINEAR );
    interpolate.addCurveData( timeSteps, valuesA );
    interpolate.addCurveData( timeSteps, valuesB );
    interpolate.computeInterpolatedValues( true );

    auto interpolatedTimeSteps = interpolate.allXValues();
    auto intervals             = interpolate.validIntervalsForAllXValues();

    EXPECT_EQ( 10, static_cast<int>( interpolatedTimeSteps.size() ) );
    EXPECT_EQ( 3, static_cast<int>( intervals.size() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaTimeHistoryCurveMergerTest, ExtractIntervalsWithSameTimeStepsOneComplete )
{
    std::vector<double> valuesA{ 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0 };
    std::vector<double> valuesB{ 10, 20, 30, HUGE_VAL, 50, HUGE_VAL, 70 };

    EXPECT_EQ( valuesA.size(), valuesB.size() );

    std::vector<time_t> timeSteps;
    for ( size_t i = 0; i < 7; i++ )
    {
        timeSteps.push_back( i );
    }

    RiaTimeHistoryCurveMerger interpolate( RiaCurveDefines::InterpolationMethod::LINEAR );
    interpolate.addCurveData( timeSteps, valuesA );
    interpolate.addCurveData( timeSteps, valuesB );
    interpolate.computeInterpolatedValues( true );

    auto interpolatedTimeSteps = interpolate.allXValues();
    auto intervals             = interpolate.validIntervalsForAllXValues();

    EXPECT_EQ( 7, static_cast<int>( interpolatedTimeSteps.size() ) );
    EXPECT_EQ( 3, static_cast<int>( intervals.size() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaTimeHistoryCurveMergerTest, ExtractIntervalsWithSameTimeStepsBothComplete )
{
    std::vector<double> valuesA{ 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0 };
    std::vector<double> valuesB{ 10, 20, 30, 40, 50, 60, 70 };

    EXPECT_EQ( valuesA.size(), valuesB.size() );

    std::vector<time_t> timeSteps;
    for ( size_t i = 0; i < 7; i++ )
    {
        timeSteps.push_back( i );
    }

    RiaTimeHistoryCurveMerger interpolate( RiaCurveDefines::InterpolationMethod::LINEAR );
    interpolate.addCurveData( timeSteps, valuesA );
    interpolate.addCurveData( timeSteps, valuesB );
    interpolate.computeInterpolatedValues( true );

    auto interpolatedTimeSteps = interpolate.allXValues();
    auto intervals             = interpolate.validIntervalsForAllXValues();

    EXPECT_EQ( 7, static_cast<int>( interpolatedTimeSteps.size() ) );
    EXPECT_EQ( 1, static_cast<int>( intervals.size() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaTimeHistoryCurveMergerTest, OverlappintTimes )
{
    std::vector<double> valuesA{ 1, 2, 3, 4, 5 };
    std::vector<double> valuesB{ 10, 20, 30, 40, 50 };

    EXPECT_EQ( valuesA.size(), valuesB.size() );

    std::vector<time_t> timeStepsA{ 0, 10, 11, 15, 20 };
    std::vector<time_t> timeStepsB{ 1, 2, 3, 5, 7 };

    RiaTimeHistoryCurveMerger interpolate( RiaCurveDefines::InterpolationMethod::LINEAR );
    interpolate.addCurveData( timeStepsA, valuesA );
    interpolate.addCurveData( timeStepsB, valuesB );
    interpolate.computeInterpolatedValues( true );
    EXPECT_EQ( 2, static_cast<int>( interpolate.curveCount() ) );

    auto interpolatedTimeSteps = interpolate.allXValues();
    auto intervals             = interpolate.validIntervalsForAllXValues();

    EXPECT_EQ( 10, static_cast<int>( interpolatedTimeSteps.size() ) );
    EXPECT_EQ( 1, static_cast<int>( intervals.size() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaTimeHistoryCurveMergerTest, RobustUse )
{
    {
        RiaTimeHistoryCurveMerger curveMerger( RiaCurveDefines::InterpolationMethod::LINEAR );
        curveMerger.computeInterpolatedValues( true );
        EXPECT_EQ( 0, static_cast<int>( curveMerger.allXValues().size() ) );
    }

    std::vector<double> valuesA{ 1, 2, 3, 4, 5 };
    std::vector<double> valuesB{ 10, 20, 30 };

    std::vector<time_t> timeStepsA{ 0, 10, 11, 15, 20 };
    std::vector<time_t> timeStepsB{ 1, 2, 3 };

    {
        RiaTimeHistoryCurveMerger curveMerger( RiaCurveDefines::InterpolationMethod::LINEAR );
        curveMerger.addCurveData( timeStepsA, valuesA );
        curveMerger.computeInterpolatedValues( true );
        EXPECT_EQ( timeStepsA.size(), curveMerger.allXValues().size() );
        EXPECT_EQ( timeStepsA.size(), curveMerger.interpolatedYValuesForAllXValues( 0 ).size() );
    }

    {
        RiaTimeHistoryCurveMerger curveMerger( RiaCurveDefines::InterpolationMethod::LINEAR );
        curveMerger.addCurveData( timeStepsA, valuesA );
        curveMerger.addCurveData( timeStepsB, valuesB );

        // Execute interpolation twice is allowed
        curveMerger.computeInterpolatedValues( true );
        curveMerger.computeInterpolatedValues( true );
        EXPECT_EQ( 8, static_cast<int>( curveMerger.allXValues().size() ) );
        EXPECT_EQ( 8, static_cast<int>( curveMerger.interpolatedYValuesForAllXValues( 0 ).size() ) );
        EXPECT_EQ( 8, static_cast<int>( curveMerger.interpolatedYValuesForAllXValues( 1 ).size() ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaTimeHistoryCurveMergerTest, NoTimeStepOverlap )
{
    std::vector<double> valuesA{ 1, 2, 3, 4, 5 };
    std::vector<double> valuesB{ 10, 20, 30 };

    std::vector<time_t> timeStepsA{ 0, 10, 11, 15, 20 };
    std::vector<time_t> timeStepsB{ 100, 200, 300 };

    {
        RiaTimeHistoryCurveMerger curveMerger( RiaCurveDefines::InterpolationMethod::LINEAR );
        curveMerger.addCurveData( timeStepsA, valuesA );
        curveMerger.addCurveData( timeStepsB, valuesB );

        // Execute interpolation twice is allowed
        curveMerger.computeInterpolatedValues( true );
        EXPECT_EQ( 8, static_cast<int>( curveMerger.allXValues().size() ) );
        EXPECT_EQ( 0, static_cast<int>( curveMerger.validIntervalsForAllXValues().size() ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaTimeHistoryCurveMergerTest, SharedXValues )
{
    std::vector<double> valuesA{ 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0 };
    std::vector<double> valuesB{ 10, 20, 30, 40, 50, 60, 70 };
    std::vector<time_t> timeSteps{ 1, 2, 3, 4, 5, 6, 7 };

    RiaTimeHistoryCurveMerger interpolate( RiaCurveDefines::InterpolationMethod::LINEAR );
    interpolate.addCurveData( timeSteps, valuesA );
    interpolate.addCurveData( timeSteps, valuesB );
    interpolate.computeInterpolatedValues( true );

    auto interpolatedTimeSteps = interpolate.allXValues();
    EXPECT_TRUE( std::equal( timeSteps.begin(), timeSteps.end(), interpolatedTimeSteps.begin() ) );

    auto generatedYValuesA = interpolate.interpolatedYValuesForAllXValues( 0 );
    EXPECT_TRUE( std::equal( valuesA.begin(), valuesA.end(), generatedYValuesA.begin() ) );

    auto generatedYValuesB = interpolate.interpolatedYValuesForAllXValues( 1 );
    EXPECT_TRUE( std::equal( valuesB.begin(), valuesB.end(), generatedYValuesB.begin() ) );
}

//--------------------------------------------------------------------------------------------------
/// A curve where the value count is smaller than the time step count must not read out of bounds.
/// See https://github.com/OPM/ResInsight/issues/12810
//--------------------------------------------------------------------------------------------------
TEST( RiaTimeHistoryCurveMergerTest, FewerValuesThanTimeSteps )
{
    std::vector<time_t> timeSteps{ 1, 2, 3, 4, 5, 6, 7 };
    std::vector<double> partialValues{ 1.0, 2.0, 3.0, 4.0 };

    RiaTimeHistoryCurveMerger interpolate( RiaCurveDefines::InterpolationMethod::LINEAR );
    interpolate.addCurveData( timeSteps, partialValues );
    interpolate.computeInterpolatedValues( true );

    // The trailing time steps without a value are discarded
    EXPECT_EQ( partialValues.size(), interpolate.allXValues().size() );

    auto generatedValues = interpolate.interpolatedYValuesForAllXValues( 0 );
    ASSERT_EQ( partialValues.size(), generatedValues.size() );
    EXPECT_TRUE( std::equal( partialValues.begin(), partialValues.end(), generatedValues.begin() ) );
}

//--------------------------------------------------------------------------------------------------
/// Curves sharing time steps, but where one curve has fewer values, must not read out of bounds in
/// the shared-X code path. See https://github.com/OPM/ResInsight/issues/12810
//--------------------------------------------------------------------------------------------------
TEST( RiaTimeHistoryCurveMergerTest, SharedXValuesWithFewerValuesInOneCurve )
{
    std::vector<time_t> timeSteps{ 1, 2, 3, 4, 5, 6, 7 };
    std::vector<double> completeValues{ 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0 };
    std::vector<double> partialValues{ 10.0, 20.0, 30.0, 40.0 };

    RiaTimeHistoryCurveMerger interpolate( RiaCurveDefines::InterpolationMethod::LINEAR );
    interpolate.addCurveData( timeSteps, completeValues );
    interpolate.addCurveData( timeSteps, partialValues );
    interpolate.computeInterpolatedValues( true );

    EXPECT_EQ( timeSteps.size(), interpolate.allXValues().size() );

    auto generatedComplete = interpolate.interpolatedYValuesForAllXValues( 0 );
    ASSERT_EQ( timeSteps.size(), generatedComplete.size() );
    EXPECT_TRUE( std::equal( completeValues.begin(), completeValues.end(), generatedComplete.begin() ) );

    auto generatedPartial = interpolate.interpolatedYValuesForAllXValues( 1 );
    ASSERT_EQ( timeSteps.size(), generatedPartial.size() );
    EXPECT_TRUE( std::equal( partialValues.begin(), partialValues.end(), generatedPartial.begin() ) );

    // No data is available for the time steps beyond the last value
    for ( size_t i = partialValues.size(); i < generatedPartial.size(); i++ )
    {
        EXPECT_EQ( HUGE_VAL, generatedPartial[i] );
    }
}

//--------------------------------------------------------------------------------------------------
/// The union of time steps must be sorted and without duplicates, also for curves with unsorted time steps
//--------------------------------------------------------------------------------------------------
TEST( RiaTimeHistoryCurveMergerTest, UnionOfTimeStepsWithUnsortedCurve )
{
    std::vector<time_t> timeStepsA{ 5, 1, 3 };
    std::vector<double> valuesA{ 50.0, 10.0, 30.0 };
    std::vector<time_t> timeStepsB{ 2, 3, 4 };
    std::vector<double> valuesB{ 2.0, 3.0, 4.0 };

    {
        RiaTimeHistoryCurveMerger merger( RiaCurveDefines::InterpolationMethod::LINEAR );
        merger.addCurveData( timeStepsA, valuesA );
        merger.addCurveData( timeStepsB, valuesB );
        merger.computeInterpolatedValues( true );

        EXPECT_EQ( ( std::vector<time_t>{ 1, 2, 3, 4, 5 } ), merger.allXValues() );
    }

    {
        RiaTimeHistoryCurveMerger merger( RiaCurveDefines::InterpolationMethod::LINEAR );
        merger.addCurveData( timeStepsA, valuesA );
        merger.addCurveData( timeStepsB, valuesB );
        merger.computeInterpolatedValues( false );

        EXPECT_EQ( ( std::vector<time_t>{ 2, 3, 4 } ), merger.allXValues() );
    }
}

//--------------------------------------------------------------------------------------------------
/// Merged values must be identical to values interpolated one by one, also when the merged time steps span several blocks
//--------------------------------------------------------------------------------------------------
TEST( RiaTimeHistoryCurveMergerTest, MergedValuesMatchSingleInterpolation )
{
    constexpr size_t curveCount    = 6;
    constexpr size_t timeStepCount = 1500;

    std::vector<std::vector<time_t>> timeSteps( curveCount );
    std::vector<std::vector<double>> values( curveCount );

    RiaTimeHistoryCurveMerger merger( RiaCurveDefines::InterpolationMethod::LINEAR );
    for ( size_t c = 0; c < curveCount; c++ )
    {
        for ( size_t i = 0; i < timeStepCount; i++ )
        {
            timeSteps[c].push_back( static_cast<time_t>( c * 3 + i * ( 7 + c ) ) );
            values[c].push_back( std::sin( 0.01 * static_cast<double>( i * ( c + 1 ) ) ) );
        }
        merger.addCurveData( timeSteps[c], values[c] );
    }
    merger.computeInterpolatedValues( true );

    ASSERT_GT( merger.allXValues().size(), static_cast<size_t>( 4096 ) );
    for ( size_t c = 0; c < curveCount; c++ )
    {
        const auto& merged = merger.interpolatedYValuesForAllXValues( c );
        ASSERT_EQ( merger.allXValues().size(), merged.size() );
        for ( size_t i = 0; i < merged.size(); i++ )
        {
            const double expected = RiaTimeHistoryCurveMerger::interpolatedYValue( merger.allXValues()[i],
                                                                                   timeSteps[c],
                                                                                   values[c],
                                                                                   RiaCurveDefines::InterpolationMethod::LINEAR );
            ASSERT_EQ( expected, merged[i] ) << "curve " << c << " index " << i;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Curves with different time steps are merged, which requires interpolation of all curves at all time steps. Disabled by
/// default, as timing depends on the machine. Run with --gtest_also_run_disabled_tests and OMP_NUM_THREADS=1 for stable CPU time.
//--------------------------------------------------------------------------------------------------
TEST( RiaTimeHistoryCurveMergerTest, DISABLED_performanceOfCurvesWithDifferentTimeSteps )
{
    constexpr size_t curveCount          = 100;
    constexpr size_t timeStepCount       = 5000;
    constexpr time_t secondsPerDay       = 24 * 60 * 60;
    constexpr time_t timeStepIntervalSec = 7 * secondsPerDay;

    std::vector<std::vector<time_t>> timeSteps( curveCount );
    std::vector<std::vector<double>> values( curveCount );
    for ( size_t c = 0; c < curveCount; c++ )
    {
        // Different offset and spacing per curve, so the merged time steps are not shared between curves
        const time_t offset = static_cast<time_t>( c ) * secondsPerDay;
        const time_t step   = timeStepIntervalSec + static_cast<time_t>( c % 5 ) * secondsPerDay;
        for ( size_t i = 0; i < timeStepCount; i++ )
        {
            timeSteps[c].push_back( offset + static_cast<time_t>( i ) * step );
            values[c].push_back( static_cast<double>( c ) + static_cast<double>( i ) );
        }
    }

    const auto start = std::chrono::steady_clock::now();

    RiaTimeHistoryCurveMerger merger( RiaCurveDefines::InterpolationMethod::LINEAR );
    for ( size_t c = 0; c < curveCount; c++ )
    {
        merger.addCurveData( timeSteps[c], values[c] );
    }
    const auto added = std::chrono::steady_clock::now();

    merger.computeInterpolatedValues( false );
    const auto computed = std::chrono::steady_clock::now();

    const auto addMs     = std::chrono::duration_cast<std::chrono::milliseconds>( added - start ).count();
    const auto computeMs = std::chrono::duration_cast<std::chrono::milliseconds>( computed - added ).count();
    std::cout << "Merge of " << curveCount << " curves with " << timeStepCount << " time steps. Add curves: " << addMs
              << " ms, compute interpolated values: " << computeMs << " ms, merged time steps: " << merger.allXValues().size() << "\n";

    ASSERT_EQ( curveCount, merger.curveCount() );
    ASSERT_FALSE( merger.allXValues().empty() );

    // The first curve is defined at all merged time steps, so all values are valid
    const auto& first = merger.interpolatedYValuesForAllXValues( 0 );
    ASSERT_EQ( merger.allXValues().size(), first.size() );
    EXPECT_TRUE( std::all_of( first.begin(), first.end(), []( double v ) { return v != HUGE_VAL; } ) );

    // Loose limit, to catch a quadratic computation without failing on slow build machines
    EXPECT_LT( computeMs, 60000 );
}
