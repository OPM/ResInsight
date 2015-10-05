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

#include "RimWellLogExtractionCurveImpl.h"

#include "cvfBase.h"
#include "cvfMath.h"
#include "cvfAssert.h"

#include <cmath> // Needed for HUGE_VAL on Linux

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurveImpl::validCurvePointIntervals(const std::vector<double>& depthValues, const std::vector<double>& values, std::vector< std::pair<size_t, size_t> >& intervals)
{
    std::vector< std::pair<size_t, size_t> > valuesIntervals;
    validValuesIntervals(values, valuesIntervals);

    intervals = valuesIntervals;

    // TODO: The following code does not work as expected
    // See issue #459 (original issue) and bugs #557 #560 and #561
    // Suggestion: Remove code related to filtering out depth values,
    // because we want to see the value difference at element borders

/*
    size_t intervalsCount = valuesIntervals.size();
    for (size_t intIdx = 0; intIdx < intervalsCount; intIdx++)
    {
        std::vector< std::pair<size_t, size_t> > depthValuesIntervals;
        validDepthValuesIntervals(depthValues, valuesIntervals[intIdx].first, valuesIntervals[intIdx].second, depthValuesIntervals);

        for (size_t dvintIdx = 0; dvintIdx < depthValuesIntervals.size(); dvintIdx++)
        {
            intervals.push_back(depthValuesIntervals[dvintIdx]);
        }
    }
*/
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurveImpl::validValuesIntervals(const std::vector<double>& values, std::vector< std::pair<size_t, size_t> >& intervals)
{
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

    if (startIdx >= 0 && startIdx < valueCount)
    {
        intervals.push_back(std::make_pair(startIdx, valueCount - 1));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurveImpl::validDepthValuesIntervals(const std::vector<double>& depthValues, size_t startIdx, size_t stopIdx, std::vector< std::pair<size_t, size_t> >& intervals)
{
    if (startIdx > stopIdx)
    {
        return;
    }

    if (startIdx == stopIdx || stopIdx - startIdx == 1)
    {
        intervals.push_back(std::make_pair(startIdx, stopIdx));
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
            intervals.push_back(std::make_pair(intStartIdx, vIdx));
            intStartIdx = vIdx + 1;
        }
    }

    if (intStartIdx <= stopIdx)
    {
        intervals.push_back(std::make_pair(intStartIdx, stopIdx));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurveImpl::addValuesFromIntervals(const std::vector<double>& values, std::vector< std::pair<size_t, size_t> >& intervals, std::vector<double>* filteredValues)
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
void RimWellLogExtractionCurveImpl::filteredIntervals(const std::vector< std::pair<size_t, size_t> >& intervals, std::vector< std::pair<size_t, size_t> >* fltrIntervals)
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
