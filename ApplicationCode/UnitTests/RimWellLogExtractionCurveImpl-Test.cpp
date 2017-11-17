#include "gtest/gtest.h"

#include "RigCurveDataTools.h"

#include <cmath> // Needed for HUGE_VAL on Linux
#include "QDateTime"
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


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RimWellLogExtractionCurveImplTest, DateOverlap)
{
    std::vector<std::pair<int, int>> dayIntervalsA { {5, 7}, {9, 12}, {15, 15}, {20, 30}          , {30, 31}};
    std::vector<std::pair<int, int>> dayIntervalsB { {3, 5}, {8, 13}, {15, 15}, {21, 22}, {25, 27}};

    QDateTime startDate;

    std::vector<std::pair<QDateTime, QDateTime>> intervalsA;
    for (const auto& interval : dayIntervalsA)
    {
        intervalsA.push_back(std::make_pair(startDate.addDays(interval.first), startDate.addDays(interval.second)));
    }

    std::vector<std::pair<QDateTime, QDateTime>> intervalsB;
    for (const auto& interval : dayIntervalsB)
    {
        intervalsB.push_back(std::make_pair(startDate.addDays(interval.first), startDate.addDays(interval.second)));
    }

    std::vector<std::pair<int, int>> allDayIntervals;

    for (const auto& intervalA : intervalsA)
    {
        auto intersecting = RigCurveDataInterpolationTools::intersectingValidIntervals(intervalA.first, intervalA.second, intervalsB);
        for (const auto& i : intersecting)
        {
            allDayIntervals.push_back(std::make_pair(startDate.daysTo(i.first), startDate.daysTo(i.second)));
        }
    }

    EXPECT_EQ(5, static_cast<int>(allDayIntervals.size()));

    EXPECT_EQ(5, allDayIntervals[0].first);
    EXPECT_EQ(5, allDayIntervals[0].second);

    EXPECT_EQ( 9, allDayIntervals[1].first);
    EXPECT_EQ(12, allDayIntervals[1].second);

    EXPECT_EQ(15, allDayIntervals[2].first);
    EXPECT_EQ(15, allDayIntervals[2].second);

    EXPECT_EQ(21, allDayIntervals[3].first);
    EXPECT_EQ(22, allDayIntervals[3].second);

    EXPECT_EQ(25, allDayIntervals[4].first);
    EXPECT_EQ(27, allDayIntervals[4].second);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RimWellLogExtractionCurveImplTest, TestDateInterpolation)
{
    std::vector<double> values{  2.0, 3.5, 5.0, 6.0};
    std::vector<int>      days{    1,   5,  10,  15};

    QDateTime startDate;

    std::vector<QDateTime> timeSteps;
    for (const auto& day : days)
    {
        timeSteps.push_back(startDate.addDays(day));
    }

    {
        QDateTime dt = startDate.addDays(1);
        double val = RigCurveDataInterpolationTools::interpolatedValue(dt, values, timeSteps);

        EXPECT_EQ(2.0, val);
    }

    {
        QDateTime dt = startDate.addDays(0);
        double val = RigCurveDataInterpolationTools::interpolatedValue(dt, values, timeSteps);

        EXPECT_EQ(HUGE_VAL, val);
    }

    {
        QDateTime dt = startDate.addDays(20);
        double val = RigCurveDataInterpolationTools::interpolatedValue(dt, values, timeSteps);

        EXPECT_EQ(HUGE_VAL, val);
    }

    {
        QDateTime dt = startDate.addDays(3);
        double val = RigCurveDataInterpolationTools::interpolatedValue(dt, values, timeSteps);

        EXPECT_EQ(2.75, val);
    }

}

//-------------------------------------------------------------------------------------------------- 
///  
//-------------------------------------------------------------------------------------------------- 
TEST(RimWellLogExtractionCurveImplTest, ExtractIntervalsWithSameTimeSteps) 
{ 
    std::vector<double> valuesA { HUGE_VAL, 1.0, HUGE_VAL, 2.0, 2.5,      3.0,      4.0, 5.0, 6.0, HUGE_VAL }; 
    std::vector<double> valuesB {       10,  20,       30,  40,  45, HUGE_VAL, HUGE_VAL, 5.0, 6.0, HUGE_VAL }; 
 
    EXPECT_EQ(valuesA.size(), valuesB.size()); 
 
    std::vector<int> days(10);
    std::iota(days.begin(), days.end(), 10);

    QDateTime startDate;

    std::vector<QDateTime> timeSteps;
    for (const auto& day : days)
    {
        timeSteps.push_back(startDate.addDays(day));
    }

    RigCurveDataInterpolationTools interpolate(valuesA, timeSteps, valuesB, timeSteps);

    auto values = interpolate.interpolatedCurveData();
    auto intervals = interpolate.validIntervals();
    
    EXPECT_EQ(5, static_cast<int>(values.size()));
    EXPECT_EQ(3, static_cast<int>(intervals.size()));

    EXPECT_EQ( 1.0, std::get<1>(values[0]));
    EXPECT_EQ(20.0, std::get<2>(values[0]));

    EXPECT_EQ( 2.0, std::get<1>(values[1]));
    EXPECT_EQ(40.0, std::get<2>(values[1]));

    EXPECT_EQ( 2.5, std::get<1>(values[2]));
    EXPECT_EQ(45.0, std::get<2>(values[2]));

    EXPECT_EQ(5.0, std::get<1>(values[3]));
    EXPECT_EQ(5.0, std::get<2>(values[3]));

    EXPECT_EQ(6.0, std::get<1>(values[4]));
    EXPECT_EQ(6.0, std::get<2>(values[4]));
} 

//-------------------------------------------------------------------------------------------------- 
///  
//-------------------------------------------------------------------------------------------------- 
TEST(RimWellLogExtractionCurveImplTest, ExtractIntervalsWithSameTimeStepsOneComplete) 
{ 
    std::vector<double> valuesA { 1.0, 2.0, 3.0,      4.0, 5.0,     6.0, 7.0 }; 
    std::vector<double> valuesB {  10,  20,  30, HUGE_VAL,  50, HUGE_VAL, 70 }; 
 
    EXPECT_EQ(valuesA.size(), valuesB.size()); 
 
    std::vector<int> days(7);
    std::iota(days.begin(), days.end(), 10);

    QDateTime startDate;

    std::vector<QDateTime> timeSteps;
    for (const auto& day : days)
    {
        timeSteps.push_back(startDate.addDays(day));
    }

    RigCurveDataInterpolationTools interpolate(valuesA, timeSteps, valuesB, timeSteps);

    auto values = interpolate.interpolatedCurveData();
    auto intervals = interpolate.validIntervals();
    
    EXPECT_EQ(5, static_cast<int>(values.size()));
    EXPECT_EQ(3, static_cast<int>(intervals.size()));
} 

//-------------------------------------------------------------------------------------------------- 
///  
//-------------------------------------------------------------------------------------------------- 
TEST(RimWellLogExtractionCurveImplTest, ExtractIntervalsWithSameTimeStepsBothComplete)
{
    std::vector<double> valuesA{ 1.0, 2.0, 3.0,      4.0, 5.0,     6.0, 7.0 };
    std::vector<double> valuesB{ 10,  20,  30,        40,  50,      60,  70 };

    EXPECT_EQ(valuesA.size(), valuesB.size());

    std::vector<int> days(7);
    std::iota(days.begin(), days.end(), 10);

    QDateTime startDate;

    std::vector<QDateTime> timeSteps;
    for (const auto& day : days)
    {
        timeSteps.push_back(startDate.addDays(day));
    }

    RigCurveDataInterpolationTools interpolate(valuesA, timeSteps, valuesB, timeSteps);

    auto values = interpolate.interpolatedCurveData();
    auto intervals = interpolate.validIntervals();

    EXPECT_EQ(7, static_cast<int>(values.size()));
    EXPECT_EQ(1, static_cast<int>(intervals.size()));
}

//-------------------------------------------------------------------------------------------------- 
///  
//-------------------------------------------------------------------------------------------------- 
TEST(RimWellLogExtractionCurveImplTest, OverlappintTimes)
{
    std::vector<double> valuesA{  1,  2,  3,  4,  5 };
    std::vector<double> valuesB{ 10, 20, 30, 40, 50 };

    EXPECT_EQ(valuesA.size(), valuesB.size());

    std::vector<int> daysA{ 0, 10, 11, 15, 20 };
    std::vector<int> daysB{ 1, 2, 3, 5, 7 };

    QDateTime startDate;

    std::vector<QDateTime> timeStepsA;
    for (const auto& day : daysA)
    {
        timeStepsA.push_back(startDate.addDays(day));
    }

    std::vector<QDateTime> timeStepsB;
    for (const auto& day : daysB)
    {
        timeStepsB.push_back(startDate.addDays(day));
    }


    RigCurveDataInterpolationTools interpolate(valuesA, timeStepsA, valuesB, timeStepsB);

    auto values = interpolate.interpolatedCurveData();
    auto intervals = interpolate.validIntervals();

    EXPECT_EQ(5, static_cast<int>(values.size()));
    EXPECT_EQ(1, static_cast<int>(intervals.size()));
}
