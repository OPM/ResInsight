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

#include "RigEclipseNativeStatCalc.h"

#include "RigCaseCellResultsData.h"
#include "RigStatisticsMath.h"
#include "RigWeightedMeanCalc.h"

#include <cmath> // Needed for HUGE_VAL on Linux

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseNativeStatCalc::RigEclipseNativeStatCalc( RigCaseCellResultsData*        cellResultsData,
                                                    const RigEclipseResultAddress& eclipseResultAddress )
    : m_resultsData( cellResultsData )
    , m_eclipseResultAddress( eclipseResultAddress )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseNativeStatCalc::minMaxCellScalarValues( size_t timeStepIndex, double& min, double& max )
{
    MinMaxAccumulator acc( min, max );

    std::vector<MinMaxAccumulator> threadAccumulators;
    threadTraverseCells( threadAccumulators, timeStepIndex );
    for ( const auto& threadAcc : threadAccumulators )
    {
        acc.addValue( threadAcc.min );
        acc.addValue( threadAcc.max );
    }
    min = acc.min;
    max = acc.max;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigEclipseNativeStatCalc::hasPreciseP10p90() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseNativeStatCalc::p10p90CellScalarValues( double& p10, double& p90 )
{
    PercentilAccumulator acc;

    for ( size_t timeStepIndex = 0; timeStepIndex < timeStepCount(); timeStepIndex++ )
    {
        std::vector<PercentilAccumulator> threadAccumulators;
        threadTraverseCells( threadAccumulators, timeStepIndex );
        for ( const auto& threadAcc : threadAccumulators )
        {
            acc.addData( threadAcc.values );
            acc.addData( threadAcc.values );
        }
    }

    acc.computep10p90( p10, p90 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseNativeStatCalc::p10p90CellScalarValues( size_t timeStepIndex, double& p10, double& p90 )
{
    PercentilAccumulator acc;

    std::vector<PercentilAccumulator> threadAccumulators;
    threadTraverseCells( threadAccumulators, timeStepIndex );
    for ( const auto& threadAcc : threadAccumulators )
    {
        acc.addData( threadAcc.values );
        acc.addData( threadAcc.values );
    }

    acc.computep10p90( p10, p90 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseNativeStatCalc::posNegClosestToZero( size_t timeStepIndex, double& pos, double& neg )
{
    PosNegAccumulator acc( pos, neg );

    std::vector<PosNegAccumulator> threadAccumulators;
    threadTraverseCells( threadAccumulators, timeStepIndex );
    for ( const auto& threadAcc : threadAccumulators )
    {
        acc.addValue( threadAcc.pos );
        acc.addValue( threadAcc.neg );
    }

    pos = acc.pos;
    neg = acc.neg;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseNativeStatCalc::addDataToHistogramCalculator( size_t                  timeStepIndex,
                                                             RigHistogramCalculator& histogramCalculator )
{
    traverseCells( histogramCalculator, timeStepIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseNativeStatCalc::uniqueValues( size_t timeStepIndex, std::set<int>& values )
{
    UniqueValueAccumulator acc;
    traverseCells( acc, timeStepIndex );
    values = acc.uniqueValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseNativeStatCalc::valueSumAndSampleCount( size_t timeStepIndex, double& valueSum, size_t& sampleCount )
{
    SumCountAccumulator acc( valueSum, sampleCount );
    traverseCells( acc, timeStepIndex );
    valueSum    = acc.valueSum;
    sampleCount = acc.sampleCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigEclipseNativeStatCalc::timeStepCount()
{
    return m_resultsData->timeStepCount( m_eclipseResultAddress );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseNativeStatCalc::mobileVolumeWeightedMean( size_t timeStepIndex, double& mean )
{
    RigEclipseResultAddress mobPorvAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                            RiaResultNames::mobilePoreVolumeName() );

    // For statistics result cases, the pore volume is not available, as
    // RigCaseCellResultsData::createPlaceholderResultEntries has not been executed
    if ( !m_resultsData->ensureKnownResultLoaded( mobPorvAddress ) )
    {
        return;
    }

    const std::vector<double>& weights = m_resultsData->cellScalarResults( mobPorvAddress, 0 );
    const std::vector<double>& values  = m_resultsData->cellScalarResults( m_eclipseResultAddress, timeStepIndex );

    const RigActiveCellInfo* actCellInfo = m_resultsData->activeCellInfo();

    RigWeightedMeanCalc::weightedMeanOverCells( &weights,
                                                &values,
                                                nullptr,
                                                false,
                                                actCellInfo,
                                                m_resultsData->isUsingGlobalActiveIndex( m_eclipseResultAddress ),
                                                &mean );
}
