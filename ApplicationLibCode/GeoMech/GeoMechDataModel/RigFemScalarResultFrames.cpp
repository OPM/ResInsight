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

#include "RigFemScalarResultFrames.h"
#include "RigStatisticsDataCache.h"
#include <cstdlib>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames::RigFemScalarResultFrames( int timeStepCount )
{
    m_dataForEachFrame.resize( timeStepCount );
    m_isSingleStepResult = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames::~RigFemScalarResultFrames()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemScalarResultFrames::enableAsSingleStepResult()
{
    m_isSingleStepResult = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigFemScalarResultFrames::timeStepCount() const
{
    return static_cast<int>( m_dataForEachFrame.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigFemScalarResultFrames::frameCount( int timeStep ) const
{
    if ( timeStep >= timeStepCount() ) return 0;

    return static_cast<int>( m_dataForEachFrame[timeStep].size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<float>& RigFemScalarResultFrames::frameData( int timeStep, int frameIndex )
{
    CVF_ASSERT( timeStep < timeStepCount() );

    if ( m_isSingleStepResult ) timeStep = 0;

    int availFrames = int( m_dataForEachFrame[timeStep].size() );

    // frame index == -1 means last available frame
    if ( frameIndex == -1 ) frameIndex = availFrames - 1;

    CVF_ASSERT( frameIndex >= 0 );

    if ( frameIndex >= availFrames )
    {
        m_dataForEachFrame[timeStep].resize( size_t( frameIndex + 1 ) );
    }

    return m_dataForEachFrame[timeStep][frameIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<float>& RigFemScalarResultFrames::frameData( int timeStep, int frameIndex ) const
{
    CVF_ASSERT( timeStep < timeStepCount() );

    if ( m_isSingleStepResult ) timeStep = 0;

    int availFrames = int( m_dataForEachFrame[timeStep].size() );

    // frame index == -1 means last available frame
    if ( frameIndex == -1 ) frameIndex = availFrames - 1;

    if ( frameIndex < 0 ) return m_noData;

    if ( frameIndex >= availFrames )
    {
        return m_noData;
    }

    return m_dataForEachFrame[timeStep][frameIndex];
}
