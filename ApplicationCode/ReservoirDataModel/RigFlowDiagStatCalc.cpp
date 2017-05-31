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

#include "RigFlowDiagStatCalc.h"
#include "RigFlowDiagResults.h"

#include <math.h>
#include "RigStatisticsMath.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFlowDiagStatCalc::RigFlowDiagStatCalc(RigFlowDiagResults* flowDiagResults, const RigFlowDiagResultAddress& resVarAddr)
: m_resVarAddr(resVarAddr)
{
    m_resultsData = flowDiagResults;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagStatCalc::minMaxCellScalarValues(size_t timeStepIndex, double& min, double& max)
{
    MinMaxAccumulator minMaxCalc(min, max);
    const std::vector<double>* vals = m_resultsData->resultValues(m_resVarAddr, timeStepIndex);

    if (vals) minMaxCalc.addData(*vals);

    min = minMaxCalc.min;
    max = minMaxCalc.max;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagStatCalc::posNegClosestToZero(size_t timeStepIndex, double& pos, double& neg)
{
    PosNegAccumulator posNegCalc(pos, neg);
    const std::vector<double>* vals = m_resultsData->resultValues(m_resVarAddr, timeStepIndex);

    if ( vals ) posNegCalc.addData(*vals);

    pos = posNegCalc.pos;
    neg = posNegCalc.neg;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagStatCalc::valueSumAndSampleCount(size_t timeStepIndex, double& valueSum, size_t& sampleCount)
{
    SumCountAccumulator sumCountCalc(valueSum, sampleCount);
    const std::vector<double>* vals = m_resultsData->resultValues(m_resVarAddr, timeStepIndex);

    if ( vals ) sumCountCalc.addData(*vals);

    valueSum    = sumCountCalc.valueSum;
    sampleCount = sumCountCalc.sampleCount;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagStatCalc::addDataToHistogramCalculator(size_t timeStepIndex, RigHistogramCalculator& histogramCalculator)
{
    const std::vector<double>* vals = m_resultsData->resultValues(m_resVarAddr, timeStepIndex);

    if ( vals ) histogramCalculator.addData(*vals);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagStatCalc::uniqueValues(size_t timeStepIndex, std::set<int>& uniqueValues)
{
    const std::vector<double>* vals = m_resultsData->resultValues(m_resVarAddr, timeStepIndex);

    if ( vals ) for ( double val : (*vals) ) uniqueValues.insert(static_cast<int>(val));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigFlowDiagStatCalc::timeStepCount()
{
    return m_resultsData->timeStepCount();
}


