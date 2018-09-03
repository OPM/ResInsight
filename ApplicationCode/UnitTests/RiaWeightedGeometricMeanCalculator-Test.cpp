#include "gtest/gtest.h"

#include "RiaWeightedGeometricMeanCalculator.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaWeightedGeometricMeanCalculator, BasicUsage)
{
    {
        RiaWeightedGeometricMeanCalculator calc;

        EXPECT_DOUBLE_EQ(0.0, calc.aggregatedWeight());
        EXPECT_DOUBLE_EQ(0.0, calc.weightedMean());
    }


    {
        RiaWeightedGeometricMeanCalculator calc;

        std::vector<double> values {30.0, 60.0};
        std::vector<double> weights {1.5, 3.5};

        for (size_t i = 0; i< values.size(); i++)
        {
            calc.addValueAndWeight(values[i], weights[i]);
        }

        double expectedValue = std::pow(
            std::pow(30.0, 1.5) * std::pow(60.0, 3.5),
            1 / (1.5 + 3.5)
        );

        EXPECT_DOUBLE_EQ(5.0, calc.aggregatedWeight());
        EXPECT_NEAR(expectedValue, calc.weightedMean(), 1e-10);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaWeightedGeometricMeanCalculator, BigValues)
{
    RiaWeightedGeometricMeanCalculator calc;

    std::vector<double> values{ 3000000.0, 6000000.0, 1250000, 2200000 };
    std::vector<double> weights{ 1.5, 3.5, 7, 5 };

    for (size_t i = 0; i < values.size(); i++)
    {
        calc.addValueAndWeight(values[i], weights[i]);
    }

    double expectedValue = std::pow(
        std::pow(3000000.0, 1.5) * std::pow(6000000.0, 3.5) * std::pow(1250000.0, 7) * std::pow(2200000.0, 5),
        1 / (1.5 + 3.5 + 7 + 5)
    );

    EXPECT_DOUBLE_EQ(17.0, calc.aggregatedWeight());
    EXPECT_NEAR(expectedValue, calc.weightedMean(), 1e-8);
}
