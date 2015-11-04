/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RigNativeStatCalc.h"

#include "RigStatisticsMath.h"
#include "RigCaseCellResultsData.h"
#include "RigStatisticsMath.h"

#include <cmath> // Needed for HUGE_VAL on Linux


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigNativeStatCalc::RigNativeStatCalc(RigCaseCellResultsData* cellResultsData, size_t scalarResultIndex)
    : m_resultsData(cellResultsData),
    m_scalarResultIndex(scalarResultIndex)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigNativeStatCalc::minMaxCellScalarValues(size_t timeStepIndex, double& min, double& max)
{
    std::vector<double>& values = m_resultsData->cellScalarResults(m_scalarResultIndex, timeStepIndex);

    size_t i;
    for (i = 0; i < values.size(); i++)
    {
        if (values[i] == HUGE_VAL)
        {
            continue;
        }

        if (values[i] < min)
        {
            min = values[i];
        }

        if (values[i] > max)
        {
            max = values[i];
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigNativeStatCalc::posNegClosestToZero(size_t timeStepIndex, double& pos, double& neg)
{
    std::vector<double>& values = m_resultsData->cellScalarResults(m_scalarResultIndex, timeStepIndex);

    size_t i;
    for (i = 0; i < values.size(); i++)
    {
        if (values[i] == HUGE_VAL)
        {
            continue;
        }

        if (values[i] < pos && values[i] > 0)
        {
            pos = values[i];
        }

        if (values[i] > neg && values[i] < 0)
        {
            neg = values[i];
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigNativeStatCalc::addDataToHistogramCalculator(size_t timeStepIndex, RigHistogramCalculator& histogramCalculator)
{
    std::vector<double>& values = m_resultsData->cellScalarResults(m_scalarResultIndex, timeStepIndex);

    histogramCalculator.addData(values);
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigNativeStatCalc::valueSumAndSampleCount(size_t timeStepIndex, double& valueSum, size_t& sampleCount)
{
    std::vector<double>& values = m_resultsData->cellScalarResults(m_scalarResultIndex, timeStepIndex);
    size_t undefValueCount = 0;
    for (size_t cIdx = 0; cIdx < values.size(); ++cIdx)
    {
        double value = values[cIdx];
        if (value == HUGE_VAL || value != value)
        {
            ++undefValueCount;
            continue;
        }

        valueSum += value;
    }

    sampleCount += values.size();
    sampleCount -= undefValueCount;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigNativeStatCalc::timeStepCount()
{
    return m_resultsData->timeStepCount(m_scalarResultIndex);
}
