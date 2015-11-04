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

#include "RigStatisticsDataCache.h"

#include "RigStatisticsCalculator.h"
#include "RigStatisticsMath.h"

#include <cmath> // Needed for HUGE_VAL on Linux


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigStatisticsDataCache::RigStatisticsDataCache(RigStatisticsCalculator* statisticsCalculator)
    : m_statisticsCalculator(statisticsCalculator)
{
    clearAllStatistics();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStatisticsDataCache::clearAllStatistics()
{
    m_statsAllTimesteps = StatisticsValues();
    m_statsPrTs.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStatisticsDataCache::minMaxCellScalarValues(double& min, double& max)
{
    if (!m_statsAllTimesteps.m_isMaxMinCalculated)
    {
        min = HUGE_VAL;
        max = -HUGE_VAL;
        
        size_t i;
        for (i = 0; i < m_statisticsCalculator->timeStepCount(); i++)
        {
            double tsmin, tsmax;
            this->minMaxCellScalarValues(i, tsmin, tsmax);
            if (tsmin < min) min = tsmin;
            if (tsmax > max) max = tsmax;
        }

        m_statsAllTimesteps.m_minValue = min;
        m_statsAllTimesteps.m_maxValue = max;
        m_statsAllTimesteps.m_isMaxMinCalculated = true;
    }

    min = m_statsAllTimesteps.m_minValue;
    max = m_statsAllTimesteps.m_maxValue;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStatisticsDataCache::minMaxCellScalarValues(size_t timeStepIndex, double& min, double& max)
{
    if (timeStepIndex >= m_statsPrTs.size())
    {
        m_statsPrTs.resize(timeStepIndex + 1);
    }

    if (!m_statsPrTs[timeStepIndex].m_isMaxMinCalculated)
    {
        double tsMin = HUGE_VAL;
        double tsMax = -HUGE_VAL;

        m_statisticsCalculator->minMaxCellScalarValues(timeStepIndex, tsMin, tsMax);

        m_statsPrTs[timeStepIndex].m_minValue = tsMin;
        m_statsPrTs[timeStepIndex].m_maxValue = tsMax;

        m_statsPrTs[timeStepIndex].m_isMaxMinCalculated = true;
    }

    min = m_statsPrTs[timeStepIndex].m_minValue;
    max = m_statsPrTs[timeStepIndex].m_maxValue;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStatisticsDataCache::posNegClosestToZero(double& pos, double& neg)
{
    if (!m_statsAllTimesteps.m_isClosestToZeroCalculated)
    {
        pos = HUGE_VAL;
        neg = -HUGE_VAL;
        
        size_t i;
        for (i = 0; i < m_statisticsCalculator->timeStepCount(); i++)
        {
            double tsNeg, tsPos;
            this->posNegClosestToZero(i, tsPos, tsNeg);
            if (tsNeg > neg && tsNeg < 0) neg = tsNeg;
            if (tsPos < pos && tsPos > 0) pos = tsPos;
        }

        m_statsAllTimesteps.m_posClosestToZero = pos;
        m_statsAllTimesteps.m_negClosestToZero = neg;
        m_statsAllTimesteps.m_isClosestToZeroCalculated = true;
    }

    pos = m_statsAllTimesteps.m_posClosestToZero;
    neg = m_statsAllTimesteps.m_negClosestToZero;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStatisticsDataCache::posNegClosestToZero(size_t timeStepIndex, double& posNearZero, double& negNearZero)
{
    if (timeStepIndex >= m_statsPrTs.size())
    {
        m_statsPrTs.resize(timeStepIndex + 1);
    }

    if (!m_statsPrTs[timeStepIndex].m_isClosestToZeroCalculated)
    {

        double pos = HUGE_VAL;
        double neg = -HUGE_VAL;
        
        m_statisticsCalculator->posNegClosestToZero(timeStepIndex, pos, neg);

        m_statsPrTs[timeStepIndex].m_posClosestToZero = pos;
        m_statsPrTs[timeStepIndex].m_negClosestToZero = neg;

        m_statsPrTs[timeStepIndex].m_isClosestToZeroCalculated = true;
    }

    posNearZero = m_statsPrTs[timeStepIndex].m_posClosestToZero;
    negNearZero = m_statsPrTs[timeStepIndex].m_negClosestToZero;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RigStatisticsDataCache::cellScalarValuesHistogram()
{
    computeHistogramStatisticsIfNeeded();

    return m_statsAllTimesteps.m_histogram;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RigStatisticsDataCache::cellScalarValuesHistogram(size_t timeStepIndex)
{
    computeHistogramStatisticsIfNeeded(timeStepIndex);

    return m_statsPrTs[timeStepIndex].m_histogram;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStatisticsDataCache::p10p90CellScalarValues(double& p10, double& p90)
{
    computeHistogramStatisticsIfNeeded();

    p10 = m_statsAllTimesteps.m_p10;
    p90 = m_statsAllTimesteps.m_p90;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStatisticsDataCache::p10p90CellScalarValues(size_t timeStepIndex, double& p10, double& p90)
{
    computeHistogramStatisticsIfNeeded(timeStepIndex);

    p10 = m_statsPrTs[timeStepIndex].m_p10;
    p90 = m_statsPrTs[timeStepIndex].m_p90;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStatisticsDataCache::meanCellScalarValues(double& meanValue)
{
    if (!m_statsAllTimesteps.m_isMeanCalculated)
    {
        m_statisticsCalculator->meanCellScalarValue(m_statsAllTimesteps.m_meanValue);
        m_statsAllTimesteps.m_isMeanCalculated = true;
    }

    meanValue = m_statsAllTimesteps.m_meanValue;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStatisticsDataCache::meanCellScalarValues(size_t timeStepIndex, double& meanValue)
{
   if (!m_statsPrTs[timeStepIndex].m_isMeanCalculated)
    {
        m_statisticsCalculator->meanCellScalarValue(timeStepIndex, m_statsPrTs[timeStepIndex].m_meanValue);
        m_statsPrTs[timeStepIndex].m_isMeanCalculated = true;
    }

    meanValue = m_statsPrTs[timeStepIndex].m_meanValue;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStatisticsDataCache::computeHistogramStatisticsIfNeeded()
{
    if (m_statsAllTimesteps.m_histogram.size() == 0)
    {
        double min;
        double max;
        size_t nBins = 100;
        this->minMaxCellScalarValues(min, max);

        RigHistogramCalculator histCalc(min, max, nBins, &m_statsAllTimesteps.m_histogram);

        m_statisticsCalculator->addDataToHistogramCalculator(histCalc);

        m_statsAllTimesteps.m_p10 = histCalc.calculatePercentil(0.1);
        m_statsAllTimesteps.m_p90 = histCalc.calculatePercentil(0.9);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStatisticsDataCache::computeHistogramStatisticsIfNeeded(size_t timeStepIndex)
{
   if (m_statsPrTs[timeStepIndex].m_histogram.size() == 0)
    {
        double min;
        double max;
        size_t nBins = 100;
        this->minMaxCellScalarValues(min, max);

        RigHistogramCalculator histCalc(min, max, nBins, &m_statsPrTs[timeStepIndex].m_histogram);

        m_statisticsCalculator->addDataToHistogramCalculator(timeStepIndex, histCalc);

        m_statsPrTs[timeStepIndex].m_p10 = histCalc.calculatePercentil(0.1);
        m_statsPrTs[timeStepIndex].m_p90 = histCalc.calculatePercentil(0.9);
    }
}

