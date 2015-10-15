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

#include "RigWellLogCurveData.h"

#include "cvfMath.h"
#include "cvfAssert.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigWellLogCurveData::RigWellLogCurveData()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigWellLogCurveData::~RigWellLogCurveData()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigWellLogCurveData::setPoints(const std::vector<double>& xValues, const std::vector<double>& yValues)
{
    CVF_ASSERT(xValues.size() == yValues.size());

    m_xValues = xValues;
    m_yValues = yValues;
    
    calculateValidPointsIntervals();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigWellLogCurveData::xValues() const
{
    return m_xValues;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigWellLogCurveData::yValues() const
{
    return m_yValues;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector< std::pair<size_t, size_t> >& RigWellLogCurveData::filteredIntervals() const
{
    return m_validPointsIntervals;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellLogCurveData::validXValues() const
{
    std::vector<double> filteredValues;
    getValuesByIntervals(m_xValues, m_validPointsIntervals, &filteredValues);

    return filteredValues;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellLogCurveData::validYValues() const
{
    std::vector<double> filteredValues;
    getValuesByIntervals(m_yValues, m_validPointsIntervals, &filteredValues);

    return filteredValues;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector< std::pair<size_t, size_t> > RigWellLogCurveData::validPointsIntervals() const
{
    std::vector< std::pair<size_t, size_t> > intervals;
    computeFilteredIntervals(m_validPointsIntervals, &intervals);

    return intervals;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigWellLogCurveData::calculateValidPointsIntervals()
{
    std::vector< std::pair<size_t, size_t> > valuesIntervals;
    calculateIntervalsOfValidValues(m_xValues, &valuesIntervals);

    std::vector< std::pair<size_t, size_t> > pointsIntervals;

    size_t intervalsCount = valuesIntervals.size();
    for (size_t intIdx = 0; intIdx < intervalsCount; intIdx++)
    {
        std::vector< std::pair<size_t, size_t> > depthValuesIntervals;
        calculateIntervalsOfValidDepthValues(m_yValues, valuesIntervals[intIdx].first, valuesIntervals[intIdx].second, &depthValuesIntervals);

        for (size_t dvintIdx = 0; dvintIdx < depthValuesIntervals.size(); dvintIdx++)
        {
            pointsIntervals.push_back(depthValuesIntervals[dvintIdx]);
        }
    }

    m_validPointsIntervals = pointsIntervals;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigWellLogCurveData::calculateIntervalsOfValidValues(const std::vector<double>& values, std::vector< std::pair<size_t, size_t> >* intervals)
{
    CVF_ASSERT(intervals);

    int startIdx = -1;
    size_t vIdx = 0;

    size_t valueCount = values.size();
    while (vIdx < valueCount)
    {
        double value = values[vIdx];
        if (value == HUGE_VAL || value == -HUGE_VAL || value != value)
        {
            if (startIdx >= 0)
            {
                intervals->push_back(std::make_pair(startIdx, vIdx - 1));
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
        intervals->push_back(std::make_pair(startIdx, valueCount - 1));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigWellLogCurveData::calculateIntervalsOfValidDepthValues(const std::vector<double>& depthValues, size_t startIdx, size_t stopIdx, std::vector< std::pair<size_t, size_t> >* intervals)
{
    CVF_ASSERT(intervals);

    if (startIdx > stopIdx)
    {
        return;
    }

    if (startIdx == stopIdx || stopIdx - startIdx == 1)
    {
        intervals->push_back(std::make_pair(startIdx, stopIdx));
        return;
    }

    // !! TODO: Find a reasonable tolerance
    const double depthDiffTolerance = 0.1;

    // Find intervals containing depth values that should be connected
    size_t intStartIdx = startIdx;
    for (size_t vIdx = startIdx + 1; vIdx < stopIdx; vIdx += 2)
    {
        if (cvf::Math::abs(depthValues[vIdx + 1] - depthValues[vIdx]) > depthDiffTolerance)
        {
            intervals->push_back(std::make_pair(intStartIdx, vIdx));
            intStartIdx = vIdx + 1;
        }
    }

    if (intStartIdx <= stopIdx)
    {
        intervals->push_back(std::make_pair(intStartIdx, stopIdx));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigWellLogCurveData::getValuesByIntervals(const std::vector<double>& values, const std::vector< std::pair<size_t, size_t> >& intervals, std::vector<double>* filteredValues)
{
    CVF_ASSERT(filteredValues);

    for (size_t intIdx = 0; intIdx < intervals.size(); intIdx++)
    {
        for (size_t vIdx = intervals[intIdx].first; vIdx <= intervals[intIdx].second; vIdx++)
        {
            filteredValues->push_back(values[vIdx]);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigWellLogCurveData::computeFilteredIntervals(const std::vector< std::pair<size_t, size_t> >& intervals, std::vector< std::pair<size_t, size_t> >* fltrIntervals)
{
    CVF_ASSERT(fltrIntervals);

    const size_t intervalCount = intervals.size();
    if (intervalCount < 1) return;

    size_t index = 0;
    for (size_t intIdx = 0; intIdx < intervalCount; intIdx++)
    {
        size_t intervalSize = intervals[intIdx].second - intervals[intIdx].first + 1;
        fltrIntervals->push_back(std::make_pair(index, index + intervalSize - 1));

        index += intervalSize;
    }
}
