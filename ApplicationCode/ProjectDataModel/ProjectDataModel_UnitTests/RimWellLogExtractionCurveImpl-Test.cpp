#include "gtest/gtest.h"

#include "../RimWellLogExtractionCurveImpl.h"



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RimWellLogExtractionCurveImplTest, Dummy)
{
    std::vector<double> values;
    values.push_back(0.0);
    values.push_back(1.0);
    values.push_back(1.0);

    std::vector< std::pair<size_t, size_t> > valuesIntervals;
    RimWellLogExtractionCurveImpl::validValuesIntervals(values, valuesIntervals);

    EXPECT_EQ(1, valuesIntervals.size());
}
