#include "gtest/gtest.h"

#include "RiaTimeHistoryCurveMerger.h"

#include <cmath> // Needed for HUGE_VAL on Linux

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaTimeHistoryCurveMergerTest, TestDateInterpolation)
{
    std::vector<double>    values{  2.0, 3.5, 5.0, 6.0};
    std::vector<time_t> timeSteps{    1,   5,  10,  15};

    {
        double val = RiaTimeHistoryCurveMerger::interpolationValue(1, values, timeSteps);

        EXPECT_EQ(2.0, val);
    }

    {
        double val = RiaTimeHistoryCurveMerger::interpolationValue(0, values, timeSteps);

        EXPECT_EQ(HUGE_VAL, val);
    }

    {
        double val = RiaTimeHistoryCurveMerger::interpolationValue(20, values, timeSteps);

        EXPECT_EQ(HUGE_VAL, val);
    }

    {
        double val = RiaTimeHistoryCurveMerger::interpolationValue(3, values, timeSteps);

        EXPECT_EQ(2.75, val);
    }
}

//-------------------------------------------------------------------------------------------------- 
///  
//-------------------------------------------------------------------------------------------------- 
TEST(RiaTimeHistoryCurveMergerTest, ExtractIntervalsWithSameTimeSteps) 
{ 
    std::vector<double> valuesA { HUGE_VAL, 1.0, HUGE_VAL, 2.0, 2.5,      3.0,      4.0, 5.0, 6.0, HUGE_VAL }; 
    std::vector<double> valuesB {       10,  20,       30,  40,  45, HUGE_VAL, HUGE_VAL, 5.0, 6.0, HUGE_VAL }; 
 
    EXPECT_EQ(valuesA.size(), valuesB.size()); 
 
    std::vector<time_t> timeSteps;
    for (size_t i = 0; i < 10; i++)
    {
        timeSteps.push_back(i);
    }

    RiaTimeHistoryCurveMerger interpolate;
    interpolate.addCurveData(valuesA, timeSteps);
    interpolate.addCurveData(valuesB, timeSteps);
    interpolate.computeInterpolatedValues();

    auto interpolatedTimeSteps = interpolate.allTimeSteps();
    auto intervals = interpolate.validIntervalsForAllTimeSteps();
    
    EXPECT_EQ(10, static_cast<int>(interpolatedTimeSteps.size()));
    EXPECT_EQ(3, static_cast<int>(intervals.size()));
} 

//-------------------------------------------------------------------------------------------------- 
///  
//-------------------------------------------------------------------------------------------------- 
TEST(RiaTimeHistoryCurveMergerTest, ExtractIntervalsWithSameTimeStepsOneComplete) 
{ 
    std::vector<double> valuesA { 1.0, 2.0, 3.0,      4.0, 5.0,     6.0, 7.0 }; 
    std::vector<double> valuesB {  10,  20,  30, HUGE_VAL,  50, HUGE_VAL, 70 }; 
 
    EXPECT_EQ(valuesA.size(), valuesB.size()); 
 
    std::vector<time_t> timeSteps;
    for (size_t i = 0; i < 7; i++)
    {
        timeSteps.push_back(i);
    }

    RiaTimeHistoryCurveMerger interpolate;
    interpolate.addCurveData(valuesA, timeSteps);
    interpolate.addCurveData(valuesB, timeSteps);
    interpolate.computeInterpolatedValues();

    auto interpolatedTimeSteps = interpolate.allTimeSteps();
    auto intervals = interpolate.validIntervalsForAllTimeSteps();
    
    EXPECT_EQ(7, static_cast<int>(interpolatedTimeSteps.size()));
    EXPECT_EQ(3, static_cast<int>(intervals.size()));
} 

//-------------------------------------------------------------------------------------------------- 
///  
//-------------------------------------------------------------------------------------------------- 
TEST(RiaTimeHistoryCurveMergerTest, ExtractIntervalsWithSameTimeStepsBothComplete)
{
    std::vector<double> valuesA{ 1.0, 2.0, 3.0,      4.0, 5.0,     6.0, 7.0 };
    std::vector<double> valuesB{ 10,  20,  30,        40,  50,      60,  70 };

    EXPECT_EQ(valuesA.size(), valuesB.size());

    std::vector<time_t> timeSteps;
    for (size_t i = 0; i < 7; i++)
    {
        timeSteps.push_back(i);
    }

    RiaTimeHistoryCurveMerger interpolate;
    interpolate.addCurveData(valuesA, timeSteps);
    interpolate.addCurveData(valuesB, timeSteps);
    interpolate.computeInterpolatedValues();

    auto interpolatedTimeSteps = interpolate.allTimeSteps();
    auto intervals = interpolate.validIntervalsForAllTimeSteps();

    EXPECT_EQ(7, static_cast<int>(interpolatedTimeSteps.size()));
    EXPECT_EQ(1, static_cast<int>(intervals.size()));
}

//-------------------------------------------------------------------------------------------------- 
///  
//-------------------------------------------------------------------------------------------------- 
TEST(RiaTimeHistoryCurveMergerTest, OverlappintTimes)
{
    std::vector<double> valuesA{  1,  2,  3,  4,  5 };
    std::vector<double> valuesB{ 10, 20, 30, 40, 50 };

    EXPECT_EQ(valuesA.size(), valuesB.size());

    std::vector<time_t> timeStepsA{ 0, 10, 11, 15, 20 };
    std::vector<time_t> timeStepsB{ 1, 2, 3, 5, 7 };

    RiaTimeHistoryCurveMerger interpolate;
    interpolate.addCurveData(valuesA, timeStepsA);
    interpolate.addCurveData(valuesB, timeStepsB);
    interpolate.computeInterpolatedValues();

    auto interpolatedTimeSteps = interpolate.allTimeSteps();
    auto intervals = interpolate.validIntervalsForAllTimeSteps();

    EXPECT_EQ(10, static_cast<int>(interpolatedTimeSteps.size()));
    EXPECT_EQ(1, static_cast<int>(intervals.size()));
}

//-------------------------------------------------------------------------------------------------- 
///  
//-------------------------------------------------------------------------------------------------- 
TEST(RiaTimeHistoryCurveMergerTest, RobustUse)
{
    {
        RiaTimeHistoryCurveMerger curveMerger;
        curveMerger.computeInterpolatedValues();
        EXPECT_EQ(0, static_cast<int>(curveMerger.allTimeSteps().size()));
    }

    std::vector<double> valuesA{ 1,  2,  3,  4,  5 };
    std::vector<double> valuesB{ 10, 20, 30 };

    std::vector<time_t> timeStepsA{ 0, 10, 11, 15, 20 };
    std::vector<time_t> timeStepsB{ 1, 2, 3 };

    {
        RiaTimeHistoryCurveMerger curveMerger;
        curveMerger.addCurveData(valuesA, timeStepsA);
        curveMerger.computeInterpolatedValues();
        EXPECT_EQ(timeStepsA.size(), curveMerger.allTimeSteps().size());
        EXPECT_EQ(timeStepsA.size(), curveMerger.interpolatedCurveValuesForAllTimeSteps(0).size());
    }

    {
        RiaTimeHistoryCurveMerger curveMerger;
        curveMerger.addCurveData(valuesA, timeStepsA);
        curveMerger.addCurveData(valuesB, timeStepsB);

        // Execute interpolation twice is allowed
        curveMerger.computeInterpolatedValues();
        curveMerger.computeInterpolatedValues();
        EXPECT_EQ(8, static_cast<int>(curveMerger.allTimeSteps().size()));
        EXPECT_EQ(8, static_cast<int>(curveMerger.interpolatedCurveValuesForAllTimeSteps(0).size()));
        EXPECT_EQ(8, static_cast<int>(curveMerger.interpolatedCurveValuesForAllTimeSteps(1).size()));
    }
}

//-------------------------------------------------------------------------------------------------- 
///  
//-------------------------------------------------------------------------------------------------- 
TEST(RiaTimeHistoryCurveMergerTest, NoTimeStepOverlap)
{
    std::vector<double> valuesA{ 1,  2,  3,  4,  5 };
    std::vector<double> valuesB{ 10, 20, 30 };

    std::vector<time_t> timeStepsA{ 0, 10, 11, 15, 20 };
    std::vector<time_t> timeStepsB{ 100, 200, 300 };

    {
        RiaTimeHistoryCurveMerger curveMerger;
        curveMerger.addCurveData(valuesA, timeStepsA);
        curveMerger.addCurveData(valuesB, timeStepsB);

        // Execute interpolation twice is allowed
        curveMerger.computeInterpolatedValues();
        EXPECT_EQ(8, static_cast<int>(curveMerger.allTimeSteps().size()));
        EXPECT_EQ(0, static_cast<int>(curveMerger.validIntervalsForAllTimeSteps().size()));
    }
}

