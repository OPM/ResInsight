#include "gtest/gtest.h"

#include "RiuQwtLinearScaleEngine.h"

#include "qwt_scale_div.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiuQwtLinearScaleEngine, DivideScaleWithExplicitIntervals_NormalCase )
{
    RiuQwtLinearScaleEngine engine;

    // Range 0..10 with major step 2 and minor step 1
    QwtScaleDiv scaleDiv = engine.divideScaleWithExplicitIntervals( 0.0, 10.0, 2.0, 1.0 );

    EXPECT_DOUBLE_EQ( scaleDiv.lowerBound(), 0.0 );
    EXPECT_DOUBLE_EQ( scaleDiv.upperBound(), 10.0 );

    const QList<double>& majorTicks = scaleDiv.ticks( QwtScaleDiv::MajorTick );
    EXPECT_EQ( majorTicks.size(), 6 ); // 0, 2, 4, 6, 8, 10

    const QList<double>& minorTicks = scaleDiv.ticks( QwtScaleDiv::MinorTick );
    EXPECT_EQ( minorTicks.size(), 11 ); // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiuQwtLinearScaleEngine, DivideScaleWithExplicitIntervals_ZeroMajorStep )
{
    RiuQwtLinearScaleEngine engine;

    // Zero major step should produce no major ticks (guard against division by zero)
    QwtScaleDiv scaleDiv = engine.divideScaleWithExplicitIntervals( 0.0, 10.0, 0.0, 1.0 );

    const QList<double>& majorTicks = scaleDiv.ticks( QwtScaleDiv::MajorTick );
    EXPECT_TRUE( majorTicks.isEmpty() );

    // Minor ticks should still be built
    const QList<double>& minorTicks = scaleDiv.ticks( QwtScaleDiv::MinorTick );
    EXPECT_FALSE( minorTicks.isEmpty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiuQwtLinearScaleEngine, DivideScaleWithExplicitIntervals_ZeroMinorStep )
{
    RiuQwtLinearScaleEngine engine;

    // Zero minor step should produce no minor ticks (guard against division by zero)
    QwtScaleDiv scaleDiv = engine.divideScaleWithExplicitIntervals( 0.0, 10.0, 2.0, 0.0 );

    const QList<double>& minorTicks = scaleDiv.ticks( QwtScaleDiv::MinorTick );
    EXPECT_TRUE( minorTicks.isEmpty() );

    // Major ticks should still be built
    const QList<double>& majorTicks = scaleDiv.ticks( QwtScaleDiv::MajorTick );
    EXPECT_FALSE( majorTicks.isEmpty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiuQwtLinearScaleEngine, DivideScaleWithExplicitIntervalsAndRange_RangeDiffersFromTickInterval )
{
    RiuQwtLinearScaleEngine engine;

    // Ticks cover 0..10, but the displayed range is -2..12
    QwtScaleDiv scaleDiv = engine.divideScaleWithExplicitIntervalsAndRange( 0.0, 10.0, 2.0, 1.0, -2.0, 12.0 );

    EXPECT_DOUBLE_EQ( scaleDiv.lowerBound(), -2.0 );
    EXPECT_DOUBLE_EQ( scaleDiv.upperBound(), 12.0 );

    const QList<double>& majorTicks = scaleDiv.ticks( QwtScaleDiv::MajorTick );
    EXPECT_EQ( majorTicks.size(), 6 ); // 0, 2, 4, 6, 8, 10
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiuQwtLinearScaleEngine, DivideScaleWithExplicitIntervalsAndRange_ZeroStepIntervals )
{
    RiuQwtLinearScaleEngine engine;

    // Both step intervals zero — no ticks should be built
    QwtScaleDiv scaleDiv = engine.divideScaleWithExplicitIntervalsAndRange( 0.0, 10.0, 0.0, 0.0, 0.0, 10.0 );

    const QList<double>& majorTicks = scaleDiv.ticks( QwtScaleDiv::MajorTick );
    EXPECT_TRUE( majorTicks.isEmpty() );

    const QList<double>& minorTicks = scaleDiv.ticks( QwtScaleDiv::MinorTick );
    EXPECT_TRUE( minorTicks.isEmpty() );
}
