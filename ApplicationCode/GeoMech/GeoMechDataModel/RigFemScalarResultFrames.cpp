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
#include <stdlib.h>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames::RigFemScalarResultFrames( int frameCount )
{
    m_dataForEachFrame.resize( frameCount );
    m_isSingleFrameResult = false;
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
void RigFemScalarResultFrames::enableAsSingleFrameResult()
{
    m_isSingleFrameResult = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigFemScalarResultFrames::frameCount() const
{
    return static_cast<int>( m_dataForEachFrame.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<float>& RigFemScalarResultFrames::frameData( size_t frameIndex )
{
    if ( m_isSingleFrameResult )
    {
        return m_dataForEachFrame[0];
    }
    else
    {
        return m_dataForEachFrame[frameIndex];
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<float>& RigFemScalarResultFrames::frameData( size_t frameIndex ) const
{
    if ( m_isSingleFrameResult )
    {
        return m_dataForEachFrame[0];
    }
    else
    {
        return m_dataForEachFrame[frameIndex];
    }
}
