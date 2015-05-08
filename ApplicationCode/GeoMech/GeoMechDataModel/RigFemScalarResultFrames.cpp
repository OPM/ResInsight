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

#pragma once

#include <stdlib.h>
#include "RigFemScalarResultFrames.h"
#include "RigFemNativeStatCalc.h"
#include "RigStatisticsDataCache.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames::RigFemScalarResultFrames(const std::vector<double>& frameTimes)
: m_frameTimes(frameTimes)
{
    m_dataForEachFrame.resize(m_frameTimes.size());
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
size_t RigFemScalarResultFrames::frameCount()
{
    return m_frameTimes.size();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigStatisticsDataCache* RigFemScalarResultFrames::statistics()
{
    if (m_statistics.isNull()) 
    {
        RigFemNativeStatCalc* calculator = new RigFemNativeStatCalc(this);
        m_statistics = new RigStatisticsDataCache(calculator);
    }

    return m_statistics.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<float>& RigFemScalarResultFrames::frameData(size_t frameIndex)
{
    return m_dataForEachFrame[frameIndex];
}
