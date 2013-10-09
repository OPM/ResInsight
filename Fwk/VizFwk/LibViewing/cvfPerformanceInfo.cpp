//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#include "cvfBase.h"
#include "cvfPerformanceInfo.h"
#include "cvfMath.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::PerformanceInfo
/// \ingroup Viewing
///
/// A class for storing performance info related to rendering of the contents of a view
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Clear all data
//--------------------------------------------------------------------------------------------------
PerformanceInfo::PerformanceInfo()
{
    clear();
}


//--------------------------------------------------------------------------------------------------
/// Clear all data
//--------------------------------------------------------------------------------------------------
void PerformanceInfo::clear()
{
    resetCurrentTimers();

    int i;
    for (i = 0; i < NUM_PERFORMANCE_HISTORY_ITEMS; i++)
    {
        m_totalDrawTimeHistory[i] = UNDEFINED_DOUBLE;
    }

    m_nextHistoryItem = 0;
}


//--------------------------------------------------------------------------------------------------
/// Clear all data
//--------------------------------------------------------------------------------------------------
void PerformanceInfo::resetCurrentTimers()
{
    totalDrawTime = 0.0;
    computeVisiblePartsTime = 0.0;
    buildRenderQueueTime = 0.0;
    sortRenderQueueTime = 0.0;
    renderEngineTime = 0.0;
    visiblePartsCount = 0;
    renderedPartsCount = 0;
    vertexCount = 0;
    triangleCount = 0;
    openGLPrimitiveCount = 0;
    applyRenderStateCount = 0;
    shaderProgramChangesCount = 0;
}


//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
void PerformanceInfo::update(const PerformanceInfo& perf)
{
    totalDrawTime               += perf.totalDrawTime;
    computeVisiblePartsTime     += perf.computeVisiblePartsTime;
    buildRenderQueueTime        += perf.buildRenderQueueTime;
    sortRenderQueueTime         += perf.sortRenderQueueTime;
    renderEngineTime            += perf.renderEngineTime;
    visiblePartsCount           += perf.visiblePartsCount;
    renderedPartsCount          += perf.renderedPartsCount;
    vertexCount                 += perf.vertexCount;
    triangleCount               += perf.triangleCount;
    openGLPrimitiveCount        += perf.openGLPrimitiveCount;
    applyRenderStateCount       += perf.applyRenderStateCount;
    shaderProgramChangesCount   += perf.shaderProgramChangesCount;

    CVF_ASSERT(m_nextHistoryItem >= 0 && m_nextHistoryItem < NUM_PERFORMANCE_HISTORY_ITEMS);
    m_totalDrawTimeHistory[m_nextHistoryItem] = totalDrawTime;
    m_nextHistoryItem++;
    if (m_nextHistoryItem >= NUM_PERFORMANCE_HISTORY_ITEMS) m_nextHistoryItem = 0;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double PerformanceInfo::averageTotalDrawTime() const
{
    double sum = 0.0;
    int count = 0;

    int i;
    for (i = 0; i < NUM_PERFORMANCE_HISTORY_ITEMS; i++)
    {
        if (m_totalDrawTimeHistory[i] != UNDEFINED_DOUBLE)
        {
            sum += m_totalDrawTimeHistory[i];
            count++;
        }
    }

    if (count < 1)
    {
        return 0.0;
    }

    return sum/count;
}


} // namespace cvf

