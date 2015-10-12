#include "gtest/gtest.h"

#include "../RimWellLogExtractionCurveImpl.h"

#include <cmath> // Needed for HUGE_VAL on Linux


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

    std::vector< std::pair<size_t, size_t> > valuesIntervals;
    RimWellLogExtractionCurveImpl::calculateIntervalsOfValidValues(values, valuesIntervals);

    EXPECT_EQ(1, valuesIntervals.size());
    EXPECT_EQ(2, valuesIntervals[0].first);
    EXPECT_EQ(4, valuesIntervals[0].second);
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

    std::vector< std::pair<size_t, size_t> > valuesIntervals;
    RimWellLogExtractionCurveImpl::calculateIntervalsOfValidValues(values, valuesIntervals);

    EXPECT_EQ(2, valuesIntervals.size());
    EXPECT_EQ(2, valuesIntervals[0].first);
    EXPECT_EQ(2, valuesIntervals[0].second);
    EXPECT_EQ(5, valuesIntervals[1].first);
    EXPECT_EQ(6, valuesIntervals[1].second);
}
