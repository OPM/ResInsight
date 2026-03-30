#include "gtest/gtest.h"

#include "RiaWeightedHarmonicMeanCalculator.h"
#include <cmath>

#include <numeric>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaWeightedHarmonicMeanCalculator, BasicUsage )
{
    {
        RiaWeightedHarmonicMeanCalculator calc;

        EXPECT_DOUBLE_EQ( 0.0, calc.aggregatedWeight() );
        EXPECT_FALSE( calc.validAggregatedWeight() );
    }

    {
        RiaWeightedHarmonicMeanCalculator calc;

        std::vector<double> values{ 1, 4, 4 };
        std::vector<double> weights{ 1, 1, 1 };

        for ( size_t i = 0; i < values.size(); i++ )
        {
            calc.addValueAndWeight( values[i], weights[i] );
        }

        double expectedValue = 2.0;

        EXPECT_DOUBLE_EQ( 3.0, calc.aggregatedWeight() );
        EXPECT_NEAR( expectedValue, calc.weightedMean(), 1e-10 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaWeightedHarmonicMeanCalculator, WeightedValues )
{
    {
        RiaWeightedHarmonicMeanCalculator calc;

        std::vector<double> values{ 10, 5, 4, 3 };
        std::vector<double> weights{ 10, 5, 4, 3 };

        for ( size_t i = 0; i < values.size(); i++ )
        {
            calc.addValueAndWeight( values[i], weights[i] );
        }

        double sumWeights = std::accumulate( weights.begin(), weights.end(), 0.0 );

        EXPECT_DOUBLE_EQ( sumWeights, calc.aggregatedWeight() );
        EXPECT_NEAR( sumWeights / weights.size(), calc.weightedMean(), 1e-8 );
    }
    {
        RiaWeightedHarmonicMeanCalculator calc;

        std::vector<double> values{ 2.0, 3.0, 1.0, 4.0 };
        std::vector<double> weights{ 1.0, 2.0, 7.0, 3.0 };
        for ( size_t i = 0; i < values.size(); i++ )
        {
            calc.addValueAndWeight( values[i], weights[i] );
        }
        double sumWeights                = std::accumulate( weights.begin(), weights.end(), 0.0 );
        double aggregatedWeightAndValues = 1.0 / 2.0 + 2.0 / 3.0 + 7.0 / 1.0 + 3.0 / 4.0;
        double expectedValue             = sumWeights / aggregatedWeightAndValues;
        EXPECT_DOUBLE_EQ( sumWeights, calc.aggregatedWeight() );
        EXPECT_NEAR( expectedValue, calc.weightedMean(), 1.0e-8 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaWeightedHarmonicMeanCalculator, ZeroWeight )
{
    {
        // Zero weight only - no valid data, no crash
        RiaWeightedHarmonicMeanCalculator calc;
        calc.addValueAndWeight( 5.0, 0.0 );
        EXPECT_FALSE( calc.validAggregatedWeight() );
        EXPECT_DOUBLE_EQ( 0.0, calc.weightedMean() );
    }

    {
        // Zero weight mixed with valid data - zero weight is ignored
        RiaWeightedHarmonicMeanCalculator calc;
        calc.addValueAndWeight( 100.0, 0.0 );
        calc.addValueAndWeight( 1.0, 1.0 );
        calc.addValueAndWeight( 4.0, 1.0 );
        calc.addValueAndWeight( 4.0, 1.0 );
        calc.addValueAndWeight( 200.0, 0.0 );

        EXPECT_DOUBLE_EQ( 3.0, calc.aggregatedWeight() );
        EXPECT_NEAR( 2.0, calc.weightedMean(), 1e-10 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaWeightedHarmonicMeanCalculator, ZeroValue )
{
    // Zero value is skipped (would cause division by zero)
    RiaWeightedHarmonicMeanCalculator calc;
    calc.addValueAndWeight( 0.0, 1.0 );
    calc.addValueAndWeight( 1.0, 1.0 );
    calc.addValueAndWeight( 4.0, 1.0 );
    calc.addValueAndWeight( 4.0, 1.0 );

    EXPECT_DOUBLE_EQ( 3.0, calc.aggregatedWeight() );
    EXPECT_NEAR( 2.0, calc.weightedMean(), 1e-10 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaWeightedHarmonicMeanCalculator, NoValidData )
{
    // No valid data added - weightedMean returns 0.0 without asserting
    RiaWeightedHarmonicMeanCalculator calc;
    EXPECT_FALSE( calc.validAggregatedWeight() );
    EXPECT_DOUBLE_EQ( 0.0, calc.weightedMean() );
}
