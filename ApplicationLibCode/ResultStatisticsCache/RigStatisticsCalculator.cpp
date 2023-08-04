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

#include "RigStatisticsCalculator.h"

#include <cassert>
#include <cmath> // Needed for HUGE_VAL on Linux

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigStatisticsCalculator::meanCellScalarValue( double& meanValue )
{
    double valueSum    = 0.0;
    size_t sampleCount = 0;

    valueSumAndSampleCount( valueSum, sampleCount );

    if ( sampleCount == 0 )
    {
        meanValue = HUGE_VAL;
    }
    else
    {
        meanValue = valueSum / sampleCount;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigStatisticsCalculator::meanCellScalarValue( size_t timeStepIndex, double& meanValue )
{
    double valueSum    = 0.0;
    size_t sampleCount = 0;

    valueSumAndSampleCount( timeStepIndex, valueSum, sampleCount );

    if ( sampleCount == 0 )
    {
        meanValue = HUGE_VAL;
    }
    else
    {
        meanValue = valueSum / sampleCount;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigStatisticsCalculator::valueSumAndSampleCount( double& valueSum, size_t& sampleCount )
{
    size_t tsCount = timeStepCount();
    for ( size_t tIdx = 0; tIdx < tsCount; tIdx++ )
    {
        valueSumAndSampleCount( tIdx, valueSum, sampleCount );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigStatisticsCalculator::addDataToHistogramCalculator( RigHistogramCalculator& histogramCalculator )
{
    size_t tsCount = timeStepCount();
    for ( size_t tIdx = 0; tIdx < tsCount; tIdx++ )
    {
        addDataToHistogramCalculator( tIdx, histogramCalculator );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigStatisticsCalculator::mobileVolumeWeightedMean( size_t timeStepIndex, double& mean )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigStatisticsCalculator::mobileVolumeWeightedMean( double& mean )
{
    double sum     = 0.0;
    size_t tsCount = timeStepCount();
    for ( size_t tIdx = 0; tIdx < tsCount; tIdx++ )
    {
        double meanForTimeStep;
        mobileVolumeWeightedMean( tIdx, meanForTimeStep );
        sum += meanForTimeStep;
    }
    if ( tsCount != 0 )
    {
        mean = sum / tsCount;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigStatisticsCalculator::posNegClosestToZero( const std::vector<double>& values, double& pos, double& neg )
{
    size_t i;
    for ( i = 0; i < values.size(); i++ )
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigStatisticsCalculator::hasPreciseP10p90() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigStatisticsCalculator::p10p90CellScalarValues( double& p10, double& p90 )
{
    assert( false && "Precise p10/p90 not available" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigStatisticsCalculator::p10p90CellScalarValues( size_t timeStepIndex, double& p10, double& p90 )
{
    assert( false && "Precise p10/p90 not available" );
}
