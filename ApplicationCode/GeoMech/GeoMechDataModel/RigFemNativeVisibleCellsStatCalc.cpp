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
#include "RigFemPartResultsCollection.h"
#include "RigFemScalarResultFrames.h"

#include "RigFemPartCollection.h"
#include "RigGeoMechCaseData.h"
#include "RigStatisticsMath.h"
#include <math.h>
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemNativeVisibleCellsStatCalc::RigFemNativeVisibleCellsStatCalc( RigGeoMechCaseData*        femCase,
                                                                    const RigFemResultAddress& resVarAddr,
                                                                    const cvf::UByteArray*     cellVisibilities )
    : m_caseData( femCase )
    , m_resVarAddr( resVarAddr )
    , m_cellVisibilities( cellVisibilities )
{
    m_resultsData = femCase->femPartResults();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemNativeVisibleCellsStatCalc::minMaxCellScalarValues( size_t timeStepIndex, double& min, double& max )
{
    MinMaxAccumulator acc( min, max );
    traverseElementNodes( acc, timeStepIndex );
    min = acc.min;
    max = acc.max;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemNativeVisibleCellsStatCalc::posNegClosestToZero( size_t timeStepIndex, double& pos, double& neg )
{
    PosNegAccumulator acc( pos, neg );
    traverseElementNodes( acc, timeStepIndex );
    pos = acc.pos;
    neg = acc.neg;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemNativeVisibleCellsStatCalc::valueSumAndSampleCount( size_t timeStepIndex, double& valueSum, size_t& sampleCount )
{
    SumCountAccumulator acc( valueSum, sampleCount );
    traverseElementNodes( acc, timeStepIndex );
    valueSum    = acc.valueSum;
    sampleCount = acc.sampleCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemNativeVisibleCellsStatCalc::addDataToHistogramCalculator( size_t                  timeStepIndex,
                                                                     RigHistogramCalculator& histogramCalculator )
{
    traverseElementNodes( histogramCalculator, timeStepIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemNativeVisibleCellsStatCalc::uniqueValues( size_t timeStepIndex, std::set<int>& values )
{
    UniqueValueAccumulator acc;
    traverseElementNodes( acc, timeStepIndex );
    values = acc.uniqueValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigFemNativeVisibleCellsStatCalc::timeStepCount()
{
    return m_resultsData->frameCount();
}
