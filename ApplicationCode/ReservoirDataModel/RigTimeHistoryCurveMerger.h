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

#pragma once

#include "RigCurveDataTools.h"

#include <ctime>


//==================================================================================================
/// 
//==================================================================================================
class RigTimeHistoryCurveMerger
{
public:
    RigTimeHistoryCurveMerger();


    void                                addCurveData(const std::vector<double>& values, 
                                                     const std::vector<time_t>& timeSteps);

    void                                computeInterpolatedValues();

    RigCurveDataTools::CurveIntervals   validIntervalsForAllTimeSteps() const;
    const std::vector<time_t>&          allTimeSteps() const;
    const std::vector<double>&          interpolatedCurveValuesForAllTimeSteps(size_t curveIdx) const;

    // Non-const access is not required by any clients, but the expression parser has no available const interface
    // for specifying a data source for an expression variable. Allow non-const access to avoid copy of the contained 
    // values, interpolated for all time steps
    //
    // See ExpressionParserImpl::assignVector()
    std::vector<double>&                interpolatedCurveValuesForAllTimeSteps(size_t curveIdx);

public:
    // Helper methods, available as public to be able to access from unit tests

    static double interpolationValue(const time_t& interpolationTimeStep,
                                     const std::vector<double>& curveValues, 
                                     const std::vector<time_t>& curveTimeSteps);

private:
    void computeUnionOfTimeSteps();

private:
    std::vector<std::pair<std::vector<double>, std::vector<time_t>>> m_originalValues;

    RigCurveDataTools::CurveIntervals   m_validIntervalsForAllTimeSteps;

    std::vector<time_t>                 m_allTimeSteps;
    std::vector<std::vector<double>>    m_interpolatedValuesForAllCurves;
};
