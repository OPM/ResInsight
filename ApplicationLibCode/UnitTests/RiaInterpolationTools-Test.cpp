#include "gtest/gtest.h"

#include "RiaInterpolationTools.h"

#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaInterpolationToolsTest, LinearEmptyData )
{
    std::vector<double> x;
    std::vector<double> y;

    double res = RiaInterpolationTools::linear( x, y, 99.9 );
    EXPECT_EQ( std::numeric_limits<double>::infinity(), res );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaInterpolationToolsTest, SingleValue )
{
    std::vector<double> x = { 1.0 };
    std::vector<double> y = { 3.0 };

    double res = RiaInterpolationTools::linear( x, y, 2.0 );
    EXPECT_EQ( std::numeric_limits<double>::infinity(), res );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaInterpolationToolsTest, ValidInterval )
{
    std::vector<double> x = { 0.0, 1.0 };
    std::vector<double> y = { 0.0, 2.0 };

    double res = RiaInterpolationTools::linear( x, y, 0.5 );
    EXPECT_DOUBLE_EQ( 1.0, res );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaInterpolationToolsTest, ValidIntervalLastBin )
{
    std::vector<double> x = { 0.0, 1.0, 100.0, 1100.0 };
    std::vector<double> y = { 0.0, 2.0, 0.0, 2000.0 };

    double res = RiaInterpolationTools::linear( x, y, 600.0 );
    EXPECT_DOUBLE_EQ( 1000.0, res );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaInterpolationToolsTest, ValidIntervalValueTooLow )
{
    std::vector<double> x = { 0.0, 1.0 };
    std::vector<double> y = { 0.0, 2.0 };

    // Outside interval on low side
    double res = RiaInterpolationTools::linear( x, y, -1.0 );
    EXPECT_DOUBLE_EQ( std::numeric_limits<double>::infinity(), res );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaInterpolationToolsTest, ValidIntervalValueTooHigh )
{
    std::vector<double> x = { 0.0, 1.0 };
    std::vector<double> y = { 0.0, 2.0 };

    // Outside interval on high side
    double res = RiaInterpolationTools::linear( x, y, 100.0 );
    EXPECT_DOUBLE_EQ( std::numeric_limits<double>::infinity(), res );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaInterpolationToolsTest, ValidIntervalValueTooHighExtrapolationClosest )
{
    std::vector<double> x = { 0.0, 1.0 };
    std::vector<double> y = { 0.0, 2.0 };

    // Outside interval on high side
    double res = RiaInterpolationTools::linear( x, y, 100.0, RiaInterpolationTools::ExtrapolationMode::CLOSEST );
    EXPECT_DOUBLE_EQ( 2.0, res );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaInterpolationToolsTest, ValidIntervalValueTooLowExtrapolationClosest )
{
    std::vector<double> x = { 0.0, 1.0 };
    std::vector<double> y = { 0.0, 2.0 };

    // Outside interval on low side
    double res = RiaInterpolationTools::linear( x, y, -1.0, RiaInterpolationTools::ExtrapolationMode::CLOSEST );
    EXPECT_DOUBLE_EQ( 0.0, res );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaInterpolationToolsTest, ValidIntervalValueTooLowExtrapolationTrend )
{
    std::vector<double> x = { 0.0, 1.0 };
    std::vector<double> y = { 0.0, 2.0 };

    // Outside interval on low side
    double res = RiaInterpolationTools::linear( x, y, -1.0, RiaInterpolationTools::ExtrapolationMode::TREND );
    EXPECT_DOUBLE_EQ( -2.0, res );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaInterpolationToolsTest, ValidIntervalValueTooHighExtrapolationTrend )
{
    std::vector<double> x = { 0.0, 1.0, 2.0, 3.0 };
    std::vector<double> y = { 0.0, 1.0, 10.0, 20.0 };

    // Outside interval on low side
    double res = RiaInterpolationTools::linear( x, y, 4.0, RiaInterpolationTools::ExtrapolationMode::TREND );
    EXPECT_DOUBLE_EQ( 30.0, res );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaInterpolationToolsTest, InterpolateMissingValuesStraightLine )
{
    double              inf = std::numeric_limits<double>::infinity();
    std::vector<double> x   = { 0.0, 1.0, 2.0, 3.0, 4.0, 5.0 };
    std::vector<double> y   = { 0.0, 1.0, inf, inf, inf, 5.0 };

    RiaInterpolationTools::interpolateMissingValues( x, y );
    EXPECT_DOUBLE_EQ( y[2], 2.0 );
    EXPECT_DOUBLE_EQ( y[3], 3.0 );
    EXPECT_DOUBLE_EQ( y[4], 4.0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaInterpolationToolsTest, InterpolateMissingValuesStraightLineExtrapolateStart )
{
    double              inf = std::numeric_limits<double>::infinity();
    std::vector<double> x   = { 0.0, 1.0, 2.0, 3.0, 4.0, 5.0 };
    std::vector<double> y   = { inf, inf, 2.0, inf, 4.0, 5.0 };

    RiaInterpolationTools::interpolateMissingValues( x, y );
    EXPECT_DOUBLE_EQ( y[0], 0.0 );
    EXPECT_DOUBLE_EQ( y[1], 1.0 );
    EXPECT_DOUBLE_EQ( y[2], 2.0 );
    EXPECT_DOUBLE_EQ( y[3], 3.0 );
    EXPECT_DOUBLE_EQ( y[4], 4.0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaInterpolationToolsTest, InterpolateMissingValuesStraightLineExtrapolateEnd )
{
    double              inf = std::numeric_limits<double>::infinity();
    std::vector<double> x   = { 0.0, 1.0, 2.0, 3.0, 4.0, 5.0 };
    std::vector<double> y   = { 0.0, inf, 2.0, inf, 4.0, inf };

    RiaInterpolationTools::interpolateMissingValues( x, y );
    EXPECT_DOUBLE_EQ( y[0], 0.0 );
    EXPECT_DOUBLE_EQ( y[1], 1.0 );
    EXPECT_DOUBLE_EQ( y[2], 2.0 );
    EXPECT_DOUBLE_EQ( y[3], 3.0 );
    EXPECT_DOUBLE_EQ( y[4], 4.0 );
    EXPECT_DOUBLE_EQ( y[5], 5.0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaInterpolationToolsTest, InterpolateMissingValuesSmallDiffs )
{
    double              inf = std::numeric_limits<double>::infinity();
    std::vector<double> x   = { 898.02149910694368,
                                898.68522777852661,
                                898.68522777852661,
                                971.44605537010159,
                                971.44605537010887,
                                1074.7396805237731,
                                1074.7396805237802 };
    std::vector<double> y   = { inf, inf, inf, inf, inf, 590.65394902812329, 590.75823974609375 };

    RiaInterpolationTools::interpolateMissingValues( x, y );
    EXPECT_DOUBLE_EQ( y[0], 590.65394902812329 );
    EXPECT_DOUBLE_EQ( y[1], 590.65394902812329 );
    EXPECT_DOUBLE_EQ( y[2], 590.65394902812329 );
    EXPECT_DOUBLE_EQ( y[3], 590.65394902812329 );
    EXPECT_DOUBLE_EQ( y[4], 590.65394902812329 );
    EXPECT_DOUBLE_EQ( y[5], 590.65394902812329 );
}
