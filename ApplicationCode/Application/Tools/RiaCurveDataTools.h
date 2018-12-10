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

#pragma once

#include "cvfAssert.h"

#include <cstddef>
#include <vector>
#include <utility>
#include <set>

class QDateTime;


//==================================================================================================
/// 
//==================================================================================================
class RiaCurveDataTools
{
public:
    typedef std::vector<std::pair<size_t, size_t>> CurveIntervals;

public:
    static CurveIntervals calculateIntervalsOfValidValues(const std::vector<double>& values,
                                                          bool includePositiveValuesOnly);

    template <typename T>
    static void getValuesByIntervals(const std::vector<T>& values,
                                     const CurveIntervals& intervals,
                                     std::vector<T>* filteredValues)
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

    static std::vector<std::pair<size_t, size_t>> computePolyLineStartStopIndices(const CurveIntervals& intervals);

public:
    static bool isValidValue(double value, bool allowPositiveValuesOnly);
};

