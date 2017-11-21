#include "gtest/gtest.h"

#include "RigCurveDataTools.h"

#include <cmath> // Needed for HUGE_VAL on Linux
#include <numeric>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RimWellLogExtractionCurveImplTest, StripOffInvalidValAtEndsOfVector)
{
    std::vector<double> values;
    values.push_back(HUGE_VAL);
    values.push_back(HUGE_VAL);
    values.push_back(1.0);
    values.push_back(2.0);
    values.push_back(3.0);
    values.push_back(HUGE_VAL);

    auto valuesIntervals = RigCurveDataTools::calculateIntervalsOfValidValues(values, false);

    EXPECT_EQ(1, static_cast<int>(valuesIntervals.size()));
    EXPECT_EQ(2, static_cast<int>(valuesIntervals[0].first));
    EXPECT_EQ(4, static_cast<int>(valuesIntervals[0].second));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RimWellLogExtractionCurveImplTest, StripOffHugeValAtEndsAndInteriorOfVector)
{
    std::vector<double> values;
    values.push_back(HUGE_VAL);
    values.push_back(HUGE_VAL);
    values.push_back(1.0);
    values.push_back(HUGE_VAL);
    values.push_back(HUGE_VAL);
    values.push_back(2.0);
    values.push_back(3.0);
    values.push_back(HUGE_VAL);

    auto valuesIntervals = RigCurveDataTools::calculateIntervalsOfValidValues(values, false);

    EXPECT_EQ(2, static_cast<int>(valuesIntervals.size()));
    EXPECT_EQ(2, static_cast<int>(valuesIntervals[0].first));
    EXPECT_EQ(2, static_cast<int>(valuesIntervals[0].second));
    EXPECT_EQ(5, static_cast<int>(valuesIntervals[1].first));
    EXPECT_EQ(6, static_cast<int>(valuesIntervals[1].second));
}

