/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RigCurveDataTools.h"

#include <QDateTime>

#include <cmath> // Needed for HUGE_VAL on Linux 
#include <set>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCurveDataTools::CurveIntervals RigCurveDataTools::calculateIntervalsOfValidValues(const std::vector<double>& values,
                                                                                     bool removeNegativeValues)
{
    CurveIntervals intervals;

    int startIdx = -1;
    size_t vIdx = 0;

    size_t valueCount = values.size();
    while (vIdx < valueCount)
    {
        bool isValid = RigCurveDataTools::isValidValue(values[vIdx], removeNegativeValues);

        if (!isValid)
        {
            if (startIdx >= 0)
            {
                intervals.push_back(std::make_pair(startIdx, vIdx - 1));
                startIdx = -1;
            }
        }
        else if (startIdx < 0)
        {
            startIdx = (int)vIdx;
        }

        vIdx++;
    }

    if (startIdx >= 0 && startIdx < ((int)valueCount))
    {
        intervals.push_back(std::make_pair(startIdx, valueCount - 1));
    }

    return intervals;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<size_t, size_t>> RigCurveDataTools::computePolyLineStartStopIndices(const CurveIntervals& intervals)
{
    std::vector<std::pair<size_t, size_t>> lineStartAndStopIndices;

    const size_t intervalCount = intervals.size();
    if (intervalCount < 1) return lineStartAndStopIndices;

    size_t index = 0;
    for (size_t intIdx = 0; intIdx < intervalCount; intIdx++)
    {
        size_t intervalSize = intervals[intIdx].second - intervals[intIdx].first + 1;
        lineStartAndStopIndices.push_back(std::make_pair(index, index + intervalSize - 1));

        index += intervalSize;
    }

    return lineStartAndStopIndices;
}


//-------------------------------------------------------------------------------------------------- 
///  
//-------------------------------------------------------------------------------------------------- 
bool RigCurveDataTools::isValidValue(double value, bool removeNegativeValues)
{
    if (value == HUGE_VAL || value == -HUGE_VAL || value != value)
    {
        return false;
    }

    if (removeNegativeValues && std::signbit(value))
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigCurveDataTools::isValidValue(double value)
{
    if (value == HUGE_VAL || value == -HUGE_VAL || value != value)
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCurveDataInterpolationTools::RigCurveDataInterpolationTools(const std::vector<double>& valuesA, 
                                                               const std::vector<QDateTime>& timeStepsA, 
                                                               const std::vector<double>& valuesB, 
                                                               const std::vector<QDateTime>& timeStepsB)
    : m_valuesA(valuesA),
    m_timeStepsA(timeStepsA),
    m_valuesB(valuesB),
    m_timeStepsB(timeStepsB)
{
    computeInterpolatedValues();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCurveDataTools::CurveIntervals RigCurveDataInterpolationTools::validIntervals() const
{
    return m_curveIntervals;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::tuple<QDateTime, double, double>> RigCurveDataInterpolationTools::interpolatedCurveData() const
{
    return m_interpolatedValues;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCurveDataInterpolationTools::computeInterpolatedValues()
{
    if (m_valuesA.size() != m_timeStepsA.size() || m_valuesB.size() != m_timeStepsB.size())
    {
        return;
    }

    const bool removeNegativeValues = false;

    auto validIntervalsA = RigCurveDataTools::calculateIntervalsOfValidValues(m_valuesA, removeNegativeValues);

    std::vector<std::pair<QDateTime, QDateTime>> validTimeStepsA;
    for (const auto& interval : validIntervalsA)
    {
        validTimeStepsA.push_back(std::make_pair(m_timeStepsA[interval.first], m_timeStepsA[interval.second]));
    }

    auto validIntervalsB = RigCurveDataTools::calculateIntervalsOfValidValues(m_valuesB, removeNegativeValues);
    for (const auto& interval : validIntervalsB)
    {
        const QDateTime& from = m_timeStepsB[interval.first];
        const QDateTime& to = m_timeStepsB[interval.second];

        auto intervals = intersectingValidIntervals(from, to, validTimeStepsA);

        for (const auto& i : intervals)
        {
            std::set<QDateTime> validTimeSteps;

            // Add all time steps from curve A inside interval
            for (const auto& d : m_timeStepsA)
            {
                if (i.first <= d && d <= i.second)
                {
                    validTimeSteps.insert(d);
                }
            }

            // Add all time steps from curve B inside interval
            for (const auto& d : m_timeStepsB)
            {
                if (i.first <= d && d <= i.second)
                {
                    validTimeSteps.insert(d);
                }
            }

            size_t firstIndex = m_interpolatedValues.size();
            for (const auto& dt : validTimeSteps)
            {
                double valueA = RigCurveDataInterpolationTools::interpolatedValue(dt, m_valuesA, m_timeStepsA);
                double valueB = RigCurveDataInterpolationTools::interpolatedValue(dt, m_valuesB, m_timeStepsB);

                m_interpolatedValues.push_back(std::make_tuple(dt, valueA, valueB));
            }
            size_t lastIndex = m_interpolatedValues.size() - 1;

            m_curveIntervals.push_back(std::make_pair(firstIndex, lastIndex));
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QDateTime, QDateTime>> RigCurveDataInterpolationTools::intersectingValidIntervals(const QDateTime& a,
                                                                                           const QDateTime& b,
                                                                                           const std::vector<std::pair<QDateTime, QDateTime>>& intervals)
{
    std::vector<std::pair<QDateTime, QDateTime>> validIntervals;

    for (const auto& interval : intervals)
    {
        const QDateTime& c = interval.first;
        const QDateTime& d = interval.second;

        if (d < a)
        {
            continue;
        }

        if (b < c)
        {
            // We assume the intervals are increasing, and all other intervals are larger
            break;
        }

        if (c <= a)
        {
            if (b <= d)
            {
                validIntervals.push_back(std::make_pair(a, b));
            }
            else
            {
                validIntervals.push_back(std::make_pair(a, d));
            }
        }
        else
        {
            if (b <= d)
            {
                validIntervals.push_back(std::make_pair(c, b));
            }
            else
            {
                validIntervals.push_back(std::make_pair(c, d));
            }
        }
    }

    return validIntervals;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCurveDataInterpolationTools::interpolatedValue(const QDateTime& dt, const std::vector<double>& values, const std::vector<QDateTime>& timeSteps)
{
    if (values.size() != timeSteps.size()) return HUGE_VAL;

    for (size_t firstI = 0; firstI < timeSteps.size(); firstI++)
    {
        size_t secondI = firstI + 1;

        if (secondI == timeSteps.size())
        {
            if (timeSteps[firstI] == dt)
            {
                return values[firstI];
            }
        }

        if (secondI < timeSteps.size() &&
            timeSteps[firstI] <= dt &&
            timeSteps[secondI] > dt)
        {
            const double& firstValue = values[firstI];
            const double& secondValue = values[secondI];

            bool isFirstValid = RigCurveDataTools::isValidValue(firstValue);
            bool isSecondValid = RigCurveDataTools::isValidValue(secondValue);

            if (!isFirstValid && !isSecondValid)
            {
                CVF_ASSERT(false);

                return HUGE_VAL;
            }

            if (!isFirstValid) return secondValue;
            if (!isSecondValid) return firstValue;

            double firstDiff = timeSteps[firstI].secsTo(dt);
            double secondDiff = dt.secsTo(timeSteps[secondI]);

            double firstWeight = secondDiff / (firstDiff + secondDiff);
            double secondWeight = firstDiff / (firstDiff + secondDiff);

            double val = (firstValue * firstWeight) + (secondValue * secondWeight);

            CVF_ASSERT(RigCurveDataTools::isValidValue(val));

            return val;
        }
    }

    return HUGE_VAL;
}

