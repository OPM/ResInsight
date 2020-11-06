#include "gtest/gtest.h"

#include "RiaMedianCalculator.h"

#include <algorithm>
#include <random>
#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaMedianCalculator, EmptyDataSet )
{
    RiaMedianCalculator<double> calc;

    EXPECT_FALSE( calc.valid() );
}

TEST( RiaMedianCalculator, SingleValue )
{
    RiaMedianCalculator<double> calc;
    calc.add( 4.0 );
    EXPECT_DOUBLE_EQ( 4.0, calc.median() );
}

TEST( RiaMedianCalculator, TwoValues )
{
    RiaMedianCalculator<double> calc;

    std::vector<double> values{ 3.0, 6.0 };

    for ( size_t i = 0; i < values.size(); i++ )
    {
        calc.add( values[i] );
    }
    EXPECT_TRUE( calc.valid() );
    EXPECT_DOUBLE_EQ( 4.5, calc.median() );
}

TEST( RiaMedianCalculator, ThreeValues )
{
    RiaMedianCalculator<double> calc;

    std::vector<double> values{ 3.0, 6.0, 2.0 };

    for ( size_t i = 0; i < values.size(); i++ )
    {
        calc.add( values[i] );
    }
    EXPECT_TRUE( calc.valid() );
    EXPECT_DOUBLE_EQ( 3.0, calc.median() );
}

TEST( RiaMedianCalculator, SameValues )
{
    RiaMedianCalculator<double> calc;
    const double                value = 113.0;
    for ( size_t i = 0; i < 1000; ++i )
    {
        calc.add( value );
    }
    EXPECT_TRUE( calc.valid() );
    EXPECT_DOUBLE_EQ( value, calc.median() );
}

TEST( RiaMedianCalculator, OrderedRangeOdd )
{
    RiaMedianCalculator<double> calc;
    for ( size_t i = 0; i < 101; ++i )
    {
        calc.add( (double)i );
    }
    EXPECT_TRUE( calc.valid() );
    EXPECT_DOUBLE_EQ( 50.0, calc.median() );
}

TEST( RiaMedianCalculator, OrderedRangeEven )
{
    RiaMedianCalculator<double> calc;
    for ( size_t i = 0; i < 100; ++i )
    {
        calc.add( (double)i );
    }
    EXPECT_TRUE( calc.valid() );
    EXPECT_DOUBLE_EQ( 49.5, calc.median() );
}

TEST( RiaMedianCalculator, ShuffledRangeOdd )
{
    RiaMedianCalculator<double> calc;

    std::vector<int> values;
    for ( int i = 0; i < 101; ++i )
    {
        values.push_back( i );
    }
    std::random_device rd;
    std::mt19937       g( rd() );
    std::shuffle( values.begin(), values.end(), g );
    for ( double value : values )
    {
        calc.add( value );
    }
    EXPECT_TRUE( calc.valid() );
    EXPECT_DOUBLE_EQ( 50.0, calc.median() );
}

TEST( RiaMedianCalculator, ShuffledRangeEven )
{
    RiaMedianCalculator<double> calc;

    std::vector<int> values;
    for ( int i = 0; i < 200; ++i )
    {
        values.push_back( i );
    }
    std::random_device rd;
    std::mt19937       g( rd() );
    std::shuffle( values.begin(), values.end(), g );
    for ( double value : values )
    {
        calc.add( value );
    }

    EXPECT_TRUE( calc.valid() );
    EXPECT_DOUBLE_EQ( 99.5, calc.median() );
}
