/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RigTimeHistoryCurveMerger.h"


#include <cmath> // Needed for HUGE_VAL on Linux 


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigTimeHistoryCurveMerger::RigTimeHistoryCurveMerger()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigTimeHistoryCurveMerger::addCurveData(const std::vector<double>& values, const std::vector<time_t>& timeSteps)
{
    CVF_ASSERT(values.size() == timeSteps.size());

    m_originalValues.push_back(std::make_pair(values, timeSteps));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCurveDataTools::CurveIntervals RigTimeHistoryCurveMerger::validIntervalsForAllTimeSteps() const
{
    return m_validIntervalsForAllTimeSteps;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RigTimeHistoryCurveMerger::allTimeSteps() const
{
    return m_allTimeSteps;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigTimeHistoryCurveMerger::interpolatedCurveValuesForAllTimeSteps(size_t curveIdx) const
{
    CVF_ASSERT(curveIdx < m_interpolatedValuesForAllCurves.size());

    return m_interpolatedValuesForAllCurves[curveIdx];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double>& RigTimeHistoryCurveMerger::interpolatedCurveValuesForAllTimeSteps(size_t curveIdx)
{
    CVF_ASSERT(curveIdx < m_interpolatedValuesForAllCurves.size());

    return m_interpolatedValuesForAllCurves[curveIdx];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigTimeHistoryCurveMerger::computeInterpolatedValues()
{
    m_validIntervalsForAllTimeSteps.clear();
    m_allTimeSteps.clear();
    m_interpolatedValuesForAllCurves.clear();

    computeUnionOfTimeSteps();

    const size_t curveCount = m_originalValues.size();
    if (curveCount == 0)
    {
        return;
    }

    const size_t dataValueCount = m_allTimeSteps.size();
    if (dataValueCount == 0)
    {
        return;
    }

    m_interpolatedValuesForAllCurves.resize(curveCount);

    std::vector<double> accumulatedValidValues(dataValueCount, 1.0);

    for (size_t curveIdx = 0; curveIdx < curveCount; curveIdx++)
    {
        std::vector<double>& curveValues = m_interpolatedValuesForAllCurves[curveIdx];
        curveValues.resize(dataValueCount);

        for (size_t valueIndex = 0; valueIndex < dataValueCount; valueIndex++)
        {
            double interpolValue = interpolationValue(m_allTimeSteps[valueIndex], m_originalValues[curveIdx].first, m_originalValues[curveIdx].second);
            if (!RigCurveDataTools::isValidValue(interpolValue, false))
            {
                accumulatedValidValues[valueIndex] = HUGE_VAL;
            }

            curveValues[valueIndex] = interpolValue;
        }
    }

    m_validIntervalsForAllTimeSteps = RigCurveDataTools::calculateIntervalsOfValidValues(accumulatedValidValues, false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigTimeHistoryCurveMerger::computeUnionOfTimeSteps()
{
    m_allTimeSteps.clear();

    std::set<time_t> unionOfTimeSteps;

    for (const auto& curveData : m_originalValues)
    {
        for (const auto& dt : curveData.second)
        {
            unionOfTimeSteps.insert(dt);
        }
    }

    for (const auto& dt : unionOfTimeSteps)
    {
        m_allTimeSteps.push_back(dt);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigTimeHistoryCurveMerger::interpolationValue(const time_t& interpolationTimeStep,
                                                     const std::vector<double>& curveValues, 
                                                     const std::vector<time_t>& curveTimeSteps)
{
    if (curveValues.size() != curveTimeSteps.size()) return HUGE_VAL;

    const bool removeInterpolatedValues = false;

    for (size_t firstI = 0; firstI < curveTimeSteps.size(); firstI++)
    {
        if (curveTimeSteps.at(firstI) == interpolationTimeStep)
        {
            const double& firstValue = curveValues.at(firstI);
            if (!RigCurveDataTools::isValidValue(firstValue, removeInterpolatedValues))
            {
                return HUGE_VAL;
            }

            return firstValue;
        }

        size_t secondI = firstI + 1;

        if (secondI < curveTimeSteps.size() &&
            curveTimeSteps.at(firstI) <= interpolationTimeStep &&
            curveTimeSteps.at(secondI) > interpolationTimeStep)
        {
            if (curveTimeSteps.at(secondI) == interpolationTimeStep)
            {
                const double& secondValue = curveValues.at(secondI);
                if (!RigCurveDataTools::isValidValue(secondValue, removeInterpolatedValues))
                {
                    return HUGE_VAL;
                }

                return secondValue;
            }

            const double& firstValue = curveValues.at(firstI);
            const double& secondValue = curveValues.at(secondI);

            bool isFirstValid = RigCurveDataTools::isValidValue(firstValue, removeInterpolatedValues);
            if (!isFirstValid) return HUGE_VAL;

            bool isSecondValid = RigCurveDataTools::isValidValue(secondValue, removeInterpolatedValues);
            if (!isSecondValid) return HUGE_VAL;

            double firstDiff = interpolationTimeStep - curveTimeSteps.at(firstI);
            double secondDiff = curveTimeSteps.at(secondI) - interpolationTimeStep;

            double firstWeight = secondDiff / (firstDiff + secondDiff);
            double secondWeight = firstDiff / (firstDiff + secondDiff);

            double val = (firstValue * firstWeight) + (secondValue * secondWeight);

            CVF_ASSERT(RigCurveDataTools::isValidValue(val, removeInterpolatedValues));

            return val;
        }
    }

    return HUGE_VAL;
}

