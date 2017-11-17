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
#include <tuple>

class QDateTime;


//==================================================================================================
/// 
//==================================================================================================
class RigCurveDataTools
{
public:
    typedef std::vector<std::pair<size_t, size_t>> CurveIntervals;

public:
    static CurveIntervals calculateIntervalsOfValidValues(const std::vector<double>& values,
                                                          bool removeNegativeValues);

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
    // Helper methods, available as public to be able to access from unit tests

    static bool isValidValue(double value, bool removeNegativeValues);
    static bool isValidValue(double value);
};


//==================================================================================================
/// 
//==================================================================================================
class RigCurveDataInterpolationTools
{
public:
    RigCurveDataInterpolationTools(const std::vector<double>&     valuesA,
                                   const std::vector<QDateTime>&  timeStepsA,
                                   const std::vector<double>&     valuesB,
                                   const std::vector<QDateTime>&  timeStepsB);



    RigCurveDataTools::CurveIntervals                   validIntervals() const;
    std::vector<std::tuple<QDateTime, double, double>>  interpolatedCurveData() const;



public:
    // Helper methods, available as public to be able to access from unit tests

    static std::vector<std::pair<QDateTime, QDateTime>> intersectingValidIntervals(const QDateTime& from,
                                                                                   const QDateTime& to,
                                                                                   const std::vector<std::pair<QDateTime, QDateTime>>& intervals);

    static double interpolatedValue(const QDateTime& dt,
                                    const std::vector<double>& values, 
                                    const std::vector<QDateTime>& timeSteps);

private:
    void computeInterpolatedValues();

private:
    const std::vector<double>&      m_valuesA;
    const std::vector<QDateTime>&   m_timeStepsA;
    const std::vector<double>&      m_valuesB;
    const std::vector<QDateTime>&   m_timeStepsB;

    std::vector<std::tuple<QDateTime, double, double>>  m_interpolatedValues;
    RigCurveDataTools::CurveIntervals                   m_curveIntervals;
};
