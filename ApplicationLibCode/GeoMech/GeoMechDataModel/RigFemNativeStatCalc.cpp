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

#include "RigFemNativeStatCalc.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemScalarResultFrames.h"

#include "RigStatisticsMath.h"
#include <cmath>
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemNativeStatCalc::RigFemNativeStatCalc( RigFemPartResultsCollection* femResultCollection, const RigFemResultAddress& resVarAddr )
    : m_resVarAddr( resVarAddr )
{
    m_resultsData = femResultCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemNativeStatCalc::minMaxCellScalarValues( size_t timeStepIndex, double& min, double& max )
{
    for ( int pIdx = 0; pIdx < m_resultsData->partCount(); ++pIdx )
    {
        auto frames = m_resultsData->findOrLoadScalarResult( pIdx, m_resVarAddr );

        auto [stepIdx, frameIdx]         = m_resultsData->stepListIndexToTimeStepAndDataFrameIndex( timeStepIndex );
        const std::vector<float>& values = frames->frameData( stepIdx, frameIdx );

        size_t i;
        for ( i = 0; i < values.size(); i++ )
        {
            if ( values[i] == HUGE_VAL ) // TODO
            {
                continue;
            }
            if ( values[i] < min )
            {
                min = values[i];
            }

            if ( values[i] > max )
            {
                max = values[i];
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemNativeStatCalc::posNegClosestToZero( size_t timeStepIndex, double& pos, double& neg )
{
    for ( int pIdx = 0; pIdx < m_resultsData->partCount(); ++pIdx )
    {
        auto frames = m_resultsData->findOrLoadScalarResult( pIdx, m_resVarAddr );

        auto [stepIdx, frameIdx]         = m_resultsData->stepListIndexToTimeStepAndDataFrameIndex( timeStepIndex );
        const std::vector<float>& values = frames->frameData( stepIdx, frameIdx );

        for ( size_t i = 0; i < values.size(); i++ )
        {
            if ( values[i] == HUGE_VAL )
            {
                continue;
            }

            if ( values[i] < pos && values[i] > 0 )
            {
                pos = values[i];
            }

            if ( values[i] > neg && values[i] < 0 )
            {
                neg = values[i];
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemNativeStatCalc::valueSumAndSampleCount( size_t timeStepIndex, double& valueSum, size_t& sampleCount )
{
    int partCount = m_resultsData->partCount();

    for ( int pIdx = 0; pIdx < partCount; ++pIdx )
    {
        auto frames              = m_resultsData->findOrLoadScalarResult( pIdx, m_resVarAddr );
        auto [stepIdx, frameIdx] = m_resultsData->stepListIndexToTimeStepAndDataFrameIndex( timeStepIndex );

        const std::vector<float>& values          = frames->frameData( stepIdx, frameIdx );
        size_t                    undefValueCount = 0;
        for ( size_t cIdx = 0; cIdx < values.size(); ++cIdx )
        {
            double value = values[cIdx];
            if ( value == HUGE_VAL || value != value )
            {
                ++undefValueCount;
                continue;
            }

            valueSum += value;
        }

        sampleCount += values.size();
        sampleCount -= undefValueCount;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemNativeStatCalc::addDataToHistogramCalculator( size_t timeStepIndex, RigHistogramCalculator& histogramCalculator )
{
    int partCount = m_resultsData->partCount();
    for ( int pIdx = 0; pIdx < partCount; ++pIdx )
    {
        auto frames = m_resultsData->findOrLoadScalarResult( pIdx, m_resVarAddr );

        auto [stepIdx, frameIdx]         = m_resultsData->stepListIndexToTimeStepAndDataFrameIndex( timeStepIndex );
        const std::vector<float>& values = frames->frameData( stepIdx, frameIdx );

        histogramCalculator.addData( values );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemNativeStatCalc::uniqueValues( size_t timeStepIndex, std::set<int>& values )
{
    for ( int pIdx = 0; pIdx < m_resultsData->partCount(); ++pIdx )
    {
        auto frames = m_resultsData->findOrLoadScalarResult( pIdx, m_resVarAddr );

        auto [stepIdx, frameIdx]              = m_resultsData->stepListIndexToTimeStepAndDataFrameIndex( timeStepIndex );
        const std::vector<float>& floatValues = frames->frameData( stepIdx, frameIdx );

        for ( size_t i = 0; i < floatValues.size(); i++ )
        {
            values.insert( static_cast<int>( std::floor( floatValues[i] ) ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigFemNativeStatCalc::timeStepCount()
{
    return m_resultsData->totalSteps();
}
