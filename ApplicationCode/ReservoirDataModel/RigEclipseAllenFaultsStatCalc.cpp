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

#include "RigEclipseAllenFaultsStatCalc.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigNNCData.h"
#include "RigStatisticsMath.h"
#include "RigWeightedMeanCalc.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseAllenFaultsStatCalc::RigEclipseAllenFaultsStatCalc( RigNNCData*                    cellResultsData,
                                                              const RigEclipseResultAddress& scalarResultIndex )
    : m_caseData( cellResultsData )
    , m_resultAddress( scalarResultIndex )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseAllenFaultsStatCalc::minMaxCellScalarValues( size_t timeStepIndex, double& min, double& max )
{
    MinMaxAccumulator acc( min, max );
    traverseCells( acc, timeStepIndex );
    min = acc.min;
    max = acc.max;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseAllenFaultsStatCalc::posNegClosestToZero( size_t timeStepIndex, double& pos, double& neg )
{
    PosNegAccumulator acc( pos, neg );
    traverseCells( acc, timeStepIndex );
    pos = acc.pos;
    neg = acc.neg;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseAllenFaultsStatCalc::valueSumAndSampleCount( size_t timeStepIndex, double& valueSum, size_t& sampleCount )
{
    SumCountAccumulator acc( valueSum, sampleCount );
    traverseCells( acc, timeStepIndex );
    valueSum    = acc.valueSum;
    sampleCount = acc.sampleCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseAllenFaultsStatCalc::addDataToHistogramCalculator( size_t                  timeStepIndex,
                                                                  RigHistogramCalculator& histogramCalculator )
{
    traverseCells( histogramCalculator, timeStepIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseAllenFaultsStatCalc::uniqueValues( size_t timeStepIndex, std::set<int>& values )
{
    UniqueValueAccumulator acc;
    traverseCells( acc, timeStepIndex );
    values = acc.uniqueValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigEclipseAllenFaultsStatCalc::timeStepCount()
{
    return (size_t)1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseAllenFaultsStatCalc::mobileVolumeWeightedMean( size_t timeStepIndex, double& result ) {}
