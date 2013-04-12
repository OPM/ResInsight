

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RiaStdInclude.h"
#include "gtest/gtest.h"

#include "RigStatisticsMath.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RigStatisticsMath, BasicTest)
{
    std::vector<double> values;
    values.push_back(HUGE_VAL);
    values.push_back(2788.2723335651900);
    values.push_back(-22481.0927881701000);
    values.push_back(68778.6851686236000);
    values.push_back(-76092.8157632591000);
    values.push_back(6391.97999909729003);
    values.push_back(65930.1200169780000);
    values.push_back(-27696.2320267235000);
    values.push_back(HUGE_VAL);
    values.push_back(HUGE_VAL);
    values.push_back(96161.7546348456000);
    values.push_back(73875.6716288563000);
    values.push_back(80720.4378655615000);
    values.push_back(-98649.8109937874000);
    values.push_back(99372.9362079615000);
    values.push_back(HUGE_VAL);
    values.push_back(-57020.4389966513000);

    double min, max, range, mean, stdev;
    RigStatisticsMath::calculateBasicStatistics(values, &min, &max, &range, &mean, &stdev);

    EXPECT_DOUBLE_EQ(-98649.8109937874000, min   );
    EXPECT_DOUBLE_EQ(99372.9362079615000 , max   );
    EXPECT_DOUBLE_EQ(198022.7472017490000, range );
    EXPECT_DOUBLE_EQ(16313.8051759152000 , mean  );
    EXPECT_DOUBLE_EQ(66104.391542887200  , stdev );

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RigStatisticsMath, RankPercentiles)
{
    std::vector<double> values;
    values.push_back(HUGE_VAL);
    values.push_back(2788.2723335651900);
    values.push_back(-22481.0927881701000);
    values.push_back(68778.6851686236000);
    values.push_back(-76092.8157632591000);
    values.push_back(6391.97999909729003);
    values.push_back(65930.1200169780000);
    values.push_back(-27696.2320267235000);
    values.push_back(HUGE_VAL);
    values.push_back(HUGE_VAL);
    values.push_back(96161.7546348456000);
    values.push_back(73875.6716288563000);
    values.push_back(80720.4378655615000);
    values.push_back(-98649.8109937874000);
    values.push_back(99372.9362079615000);
    values.push_back(HUGE_VAL);
    values.push_back(-57020.4389966513000);

    std::vector<double> pValPos;
    pValPos.push_back(10);
    pValPos.push_back(40);
    pValPos.push_back(50);
    pValPos.push_back(90);
    std::vector<double> pVals = RigStatisticsMath::calculateNearestRankPercentiles(values, pValPos);

    EXPECT_DOUBLE_EQ( -76092.8157632591000, pVals[0]);
    EXPECT_DOUBLE_EQ( 2788.2723335651900  , pVals[1]);
    EXPECT_DOUBLE_EQ( 6391.979999097290   , pVals[2]);
    EXPECT_DOUBLE_EQ( 96161.7546348456000 , pVals[3]);
}




//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RigStatisticsMath, HistogramPercentiles)
{
    std::vector<double> values;
    values.push_back(HUGE_VAL);
    values.push_back(2788.2723335651900);
    values.push_back(-22481.0927881701000);
    values.push_back(68778.6851686236000);
    values.push_back(-76092.8157632591000);
    values.push_back(6391.97999909729003);
    values.push_back(65930.1200169780000);
    values.push_back(-27696.2320267235000);
    values.push_back(HUGE_VAL);
    values.push_back(HUGE_VAL);
    values.push_back(96161.7546348456000);
    values.push_back(73875.6716288563000);
    values.push_back(80720.4378655615000);
    values.push_back(-98649.8109937874000);
    values.push_back(99372.9362079615000);
    values.push_back(HUGE_VAL);
    values.push_back(-57020.4389966513000);


    double min, max, range, mean, stdev;
    RigStatisticsMath::calculateBasicStatistics(values, &min, &max, &range, &mean, &stdev);

    std::vector<size_t> histogram;
    RigHistogramCalculator histCalc(min, max, 100, &histogram);
    histCalc.addData(values);
    std::vector<double> pVals;
    double p10, p50, p90;
    p10 = histCalc.calculatePercentil(0.1);
    p50 = histCalc.calculatePercentil(0.5);
    p90 = histCalc.calculatePercentil(0.9);

    EXPECT_DOUBLE_EQ( -76273.240559989776, p10);
    EXPECT_DOUBLE_EQ( 5312.1312871307755  , p50);
    EXPECT_DOUBLE_EQ( 94818.413022321271 , p90);
}
