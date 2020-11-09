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
TEST( RiaInterpolationToolsTest, InterpolateMisssingValuesStraightLine )
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
