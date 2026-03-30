#include "gtest/gtest.h"

#include "RiaWeightedMeanCalculator.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaWeightedMeanCalculator, BasicUsage )
{
    {
        RiaWeightedMeanCalculator<double> calc;

        EXPECT_DOUBLE_EQ( 0.0, calc.aggregatedWeight() );
        EXPECT_FALSE( calc.validAggregatedWeight() );
    }

    {
        RiaWeightedMeanCalculator<double> calc;

        std::vector<double> values{ 3.0, 6.0 };
        std::vector<double> weights{ 1.0, 2.0 };

        for ( size_t i = 0; i < values.size(); i++ )
        {
            calc.addValueAndWeight( values[i], weights[i] );
        }
        EXPECT_TRUE( calc.validAggregatedWeight() );
        EXPECT_DOUBLE_EQ( 3.0, calc.aggregatedWeight() );
        EXPECT_DOUBLE_EQ( 5.0, calc.weightedMean() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaWeightedMeanCalculator, ZeroWeight )
{
    {
        // Zero weight only - no valid data
        RiaWeightedMeanCalculator<double> calc;
        calc.addValueAndWeight( 5.0, 0.0 );
        EXPECT_FALSE( calc.validAggregatedWeight() );
        EXPECT_DOUBLE_EQ( 0.0, calc.aggregatedWeight() );
        EXPECT_DOUBLE_EQ( 0.0, calc.weightedMean() );
    }

    {
        // Zero weight mixed with valid data - zero weight is ignored
        RiaWeightedMeanCalculator<double> calc;
        calc.addValueAndWeight( 100.0, 0.0 );
        calc.addValueAndWeight( 5.0, 2.0 );
        calc.addValueAndWeight( 200.0, 0.0 );
        EXPECT_TRUE( calc.validAggregatedWeight() );
        EXPECT_DOUBLE_EQ( 2.0, calc.aggregatedWeight() );
        EXPECT_DOUBLE_EQ( 5.0, calc.weightedMean() );
    }

    {
        // Negative weight is also ignored
        RiaWeightedMeanCalculator<double> calc;
        calc.addValueAndWeight( 100.0, -1.0 );
        calc.addValueAndWeight( 5.0, 2.0 );
        EXPECT_TRUE( calc.validAggregatedWeight() );
        EXPECT_DOUBLE_EQ( 2.0, calc.aggregatedWeight() );
        EXPECT_DOUBLE_EQ( 5.0, calc.weightedMean() );
    }
}
