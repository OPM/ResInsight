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


#include "RigFlowDiagVisibleCellsStatCalc.h"
#include "RigActiveCellInfo.h"

#include <math.h>
#include "RigStatisticsMath.h"
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFlowDiagVisibleCellsStatCalc::RigFlowDiagVisibleCellsStatCalc(RigFlowDiagResults* resultsData,
                                                                   const RigFlowDiagResultAddress& resVarAddr, 
                                                                   const cvf::UByteArray* cellVisibilities)
: m_resultsData(resultsData), m_resVarAddr(resVarAddr), m_cellVisibilities(cellVisibilities)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagVisibleCellsStatCalc::minMaxCellScalarValues(size_t timeStepIndex, double& min, double& max)
{
    MinMaxAccumulator acc(min, max);
    traverseElementNodes(acc, timeStepIndex);
    min = acc.min;
    max = acc.max;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagVisibleCellsStatCalc::posNegClosestToZero(size_t timeStepIndex, double& pos, double& neg)
{
    PosNegAccumulator acc(pos, neg);
    traverseElementNodes(acc, timeStepIndex);
    pos = acc.pos;
    neg = acc.neg;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagVisibleCellsStatCalc::valueSumAndSampleCount(size_t timeStepIndex, double& valueSum, size_t& sampleCount)
{
    SumCountAccumulator acc(valueSum, sampleCount);
    traverseElementNodes(acc, timeStepIndex);
    valueSum = acc.valueSum;
    sampleCount = acc.sampleCount;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagVisibleCellsStatCalc::addDataToHistogramCalculator(size_t timeStepIndex, RigHistogramCalculator& histogramCalculator)
{
    traverseElementNodes(histogramCalculator, timeStepIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagVisibleCellsStatCalc::uniqueValues(size_t timeStepIndex, std::set<int>& values)
{
    UniqueValueAccumulator acc;
    traverseElementNodes(acc, timeStepIndex);
    values = acc.uniqueValues;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigFlowDiagVisibleCellsStatCalc::timeStepCount()
{
    return m_resultsData->timeStepCount();
}


