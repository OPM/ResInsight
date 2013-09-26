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
#include "cvfConstantFrameRate.h"
#include "cvfTrace.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::ConstantFrameRate
/// \ingroup Viewing
///
/// 
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ConstantFrameRate::ConstantFrameRate()
{
    m_targetFrameRate = 10.0;
    m_minNumPartsToDraw = 1;
    m_enableDistanceSorting = false;

    m_numPartsToDraw = -1.0;

    m_pixelSizeCullingWasOn = false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ConstantFrameRate::setTargetFrameRate(double targetFrameRate)
{
    CVF_ASSERT(targetFrameRate >= 0);
    m_targetFrameRate = targetFrameRate;
    if (m_targetFrameRate < 0) m_targetFrameRate = 0;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ConstantFrameRate::setMinNumPartsToDraw(int minNumParts)
{
    CVF_ASSERT(minNumParts >= 1);
    m_minNumPartsToDraw = minNumParts;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ConstantFrameRate::enableDistanceSorting(bool enableDistanceSorting)
{
    m_enableDistanceSorting = enableDistanceSorting;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double ConstantFrameRate::targetFrameRate() const
{
    return m_targetFrameRate;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int ConstantFrameRate::minNumPartsToDraw() const
{
    return m_minNumPartsToDraw;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ConstantFrameRate::isDistanceSortingEnabled() const
{
    return m_enableDistanceSorting;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ConstantFrameRate::attachRendering(Rendering* rendering)
{
    if (rendering == m_rendering.p()) return;

    if (m_rendering.notNull())
    {
        m_rendering->setRenderQueueSorter(m_previousSorter.p());
        m_rendering->clearMaxNumPartsToDraw();
        m_rendering->cullSettings()->enablePixelSizeCulling(m_pixelSizeCullingWasOn);

        m_rendering = NULL;
    }

    if (rendering)
    {
        m_previousSorter = rendering->renderQueueSorter();
        m_pixelSizeCullingWasOn = rendering->cullSettings()->isPixelSizeCullingEnabled();

        m_rendering = rendering;
        m_rendering->clearMaxNumPartsToDraw();

        RenderQueueSorterTargetFramerate* rqs = new RenderQueueSorterTargetFramerate;
        m_rendering->setRenderQueueSorter(rqs);

        // Currently, we need to enable pixel size culling in order to
        // compute the screen projected areas of the part's bounding boxes
        m_rendering->cullSettings()->enablePixelSizeCulling(true);


        m_numPartsToDraw = -1.0;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rendering* ConstantFrameRate::attachedRendering()
{
    return m_rendering.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ConstantFrameRate::adjust(double currentFrameRate)
{
    CVF_ASSERT(m_rendering.notNull());
    if (m_rendering.isNull()) return;

    const int maxNumPartsToDraw = 10000000;
    const int maxNumPartsToDistanceSort = 1000;

    const double lastDrawTime = (currentFrameRate > 0) ? 1.0/currentFrameRate : 999.0;
    const double targetDrawTime = (m_targetFrameRate > 0) ? 1.0/m_targetFrameRate : 999.0;

    // Initialize if this is first call
    if (m_numPartsToDraw < 0)
    {
        PerformanceInfo perfInfo = m_rendering->performanceInfo();
        m_numPartsToDraw = static_cast<double>(perfInfo.renderedPartsCount);
        if (m_numPartsToDraw < m_minNumPartsToDraw) m_numPartsToDraw = m_minNumPartsToDraw;
    }

    double numPartsToDistanceSort = 0;

    double absDiff = Math::abs(lastDrawTime - targetDrawTime);
    double fractionDiff = absDiff/targetDrawTime;

    if (fractionDiff > 0.05)
    {
        if (lastDrawTime > targetDrawTime)
        {
            // We need fewer parts!!
            // Ratio always > 1
            double ratio = lastDrawTime/targetDrawTime;

            m_numPartsToDraw = m_numPartsToDraw/(1 + 0.5*(ratio - 1));
            if (m_numPartsToDraw < m_minNumPartsToDraw) m_numPartsToDraw = m_minNumPartsToDraw;
        }
        else
        {
            // We need more parts!!
            // Ratio always <= 1
            double ratio = lastDrawTime/targetDrawTime;

            m_numPartsToDraw = m_numPartsToDraw/(0.5*ratio);
        }

        // Unlikely that we'll ever exceed max of 10 mill parts
        if (m_numPartsToDraw > maxNumPartsToDraw) m_numPartsToDraw = maxNumPartsToDraw;
    }

    if (m_enableDistanceSorting)
    {
        numPartsToDistanceSort = 0.2*m_numPartsToDraw;
        if (numPartsToDistanceSort > maxNumPartsToDistanceSort) numPartsToDistanceSort = maxNumPartsToDistanceSort;
    }

    Trace::show("m_numPartsToDraw=%f  numPartsToDistanceSort=%f", m_numPartsToDraw, numPartsToDistanceSort);

    RenderQueueSorterTargetFramerate* rqs = dynamic_cast<RenderQueueSorterTargetFramerate*>(m_rendering->renderQueueSorter());
    if (rqs)
    {
        rqs->setMaxNumPartsToDraw(static_cast<uint>(m_numPartsToDraw + 0.5));
        rqs->setNumPartsToDistanceSort(static_cast<uint>(numPartsToDistanceSort + 0.5));
    }

    m_rendering->setMaxNumPartsToDraw(static_cast<size_t>(m_numPartsToDraw + 0.5));
}


/*
double g_targetDrawTime = 0.05;
double g_numDistSorted  = 1000;
double g_numPartsToDRaw = 5000;


void AdjustSettingsBasedOnTargetDrawTime(double lastFrameDrawTime, Rendering* rendering)
{
    if (g_targetDrawTime <= 0) return;

    double absDiff = abs(lastFrameDrawTime - g_targetDrawTime);

    double fractionDiff = absDiff/g_targetDrawTime;

    if (fractionDiff > 0.05)
    {
        if (lastFrameDrawTime > g_targetDrawTime)
        {
            // Ratio always > 1
            double ratio = lastFrameDrawTime/g_targetDrawTime;

            g_numPartsToDRaw = g_numPartsToDRaw/(1 + 0.5*(ratio - 1));
            if (g_numPartsToDRaw < 10) g_numPartsToDRaw = 10;
        }
        else
        {
            // We need more parts!!

            // Ratio always <= 1
            double ratio = lastFrameDrawTime/g_targetDrawTime;

            g_numPartsToDRaw = g_numPartsToDRaw/(0.5*ratio);
        }

        if (g_numPartsToDRaw > 10000000)
        {
            g_numPartsToDRaw  = 10000000;
        }

        g_numDistSorted = (int)(0.2*g_numPartsToDRaw);
        if (g_numDistSorted < 1) g_numDistSorted = 1;
    }

    Trace::show("g_numPartsToDRaw=%f  g_numDistSorted=%f", g_numPartsToDRaw, g_numDistSorted);

    RenderQueueSorterTargetFramerate* rqs = dynamic_cast<RenderQueueSorterTargetFramerate*>(rendering->renderQueueSorter());
    if (rqs)
    {
        rqs->setMaxNumPartsToDraw(static_cast<size_t>(g_numPartsToDRaw + 0.5));
        rqs->setNumPartsToDistanceSort(static_cast<size_t>(g_numDistSorted + 0.5));
    }

    rendering->setMaxNumPartsToDraw(static_cast<size_t>(g_numPartsToDRaw));
}
*/


} // namespace cvf

