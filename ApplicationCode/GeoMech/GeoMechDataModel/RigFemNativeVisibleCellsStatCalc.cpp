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


#include "RigFemNativeVisibleCellsStatCalc.h"
#include "RigFemScalarResultFrames.h"
#include "RigFemPartResultsCollection.h"

#include <math.h>
#include "RigStatisticsMath.h"
#include "RigGeoMechCaseData.h"
#include "RigFemPartCollection.h"
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemNativeVisibleCellsStatCalc::RigFemNativeVisibleCellsStatCalc(RigGeoMechCaseData* femCase,
                                                                   const RigFemResultAddress& resVarAddr, 
                                                                   const cvf::UByteArray* cellVisibilities)
: m_caseData(femCase), m_resVarAddr(resVarAddr), m_cellVisibilities(cellVisibilities)
{
    m_resultsData = femCase->femPartResults();
}

class MinMaxAccumulator
{
public:
    MinMaxAccumulator(double initMin, double initMax): max(initMax), min(initMin) {}
    void addValue(double value)
    {
        if (value == HUGE_VAL) // TODO
        {
            return;
        }

        if (value < min)
        {
            min = value;
        }

        if (value > max)
        {
            max = value;
        }
    }

    double max;
    double min;
};



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemNativeVisibleCellsStatCalc::minMaxCellScalarValues(size_t timeStepIndex, double& min, double& max)
{
    MinMaxAccumulator acc(min, max);
    traverseElementNodes(acc, timeStepIndex);
    min = acc.min;
    max = acc.max;
}


class PosNegAccumulator
{
public:
    PosNegAccumulator(double initPos, double initNeg): pos(initPos), neg(initNeg) {}
    void addValue(double value)
    {
        if (value == HUGE_VAL)
        {
            return;
        }

        if (value < pos && value > 0)
        {
            pos = value;
        }

        if (value > neg && value < 0)
        {
            neg = value;
        }
    }

    double pos;
    double neg;
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemNativeVisibleCellsStatCalc::posNegClosestToZero(size_t timeStepIndex, double& pos, double& neg)
{
    PosNegAccumulator acc(pos, neg);
    traverseElementNodes(acc, timeStepIndex);
    pos = acc.pos;
    neg = acc.neg;

}

class SumCountAccumulator
{
public:
    SumCountAccumulator(double initSum, size_t initCount): valueSum(initSum), sampleCount(initCount) {}

    void addValue(double value)
    {
        if (value == HUGE_VAL || value != value)
        {
            return;
        }

        valueSum += value;
        ++sampleCount;
    }

    double valueSum;
    size_t sampleCount;
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemNativeVisibleCellsStatCalc::valueSumAndSampleCount(size_t timeStepIndex, double& valueSum, size_t& sampleCount)
{
    SumCountAccumulator acc(valueSum, sampleCount);
    traverseElementNodes(acc, timeStepIndex);
    valueSum = acc.valueSum;
    sampleCount = acc.sampleCount;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemNativeVisibleCellsStatCalc::addDataToHistogramCalculator(size_t timeStepIndex, RigHistogramCalculator& histogramCalculator)
{
    traverseElementNodes(histogramCalculator, timeStepIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigFemNativeVisibleCellsStatCalc::timeStepCount()
{
    return m_resultsData->frameCount();
}


