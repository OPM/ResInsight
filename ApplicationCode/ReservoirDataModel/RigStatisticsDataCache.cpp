/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA, Ceetron Solutions AS
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
    m_minValue = HUGE_VAL;
    m_maxValue = -HUGE_VAL;
    m_posClosestToZero = -HUGE_VAL;
    m_negClosestToZero = HUGE_VAL;
    m_p10 = HUGE_VAL;
    m_p90 = HUGE_VAL;
    m_meanValue = HUGE_VAL;

    m_histogram.clear();
    m_maxMinValuesPrTs.clear();
    m_posNegClosestToZeroPrTs.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStatisticsDataCache::minMaxCellScalarValues(double& min, double& max)
{
	if (m_minValue == HUGE_VAL)
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

		m_minValue = min;
		m_maxValue = max;
	}

	min = m_minValue;
	max = m_maxValue;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStatisticsDataCache::minMaxCellScalarValues(size_t timeStepIndex, double& min, double& max)
{
	if (timeStepIndex >= m_maxMinValuesPrTs.size())
	{
		m_maxMinValuesPrTs.resize(timeStepIndex + 1, std::make_pair(HUGE_VAL, -HUGE_VAL));
	}

	if (m_maxMinValuesPrTs[timeStepIndex].first == HUGE_VAL)
	{
        min = HUGE_VAL;
        max = -HUGE_VAL;

        m_statisticsCalculator->minMaxCellScalarValues(timeStepIndex, min, max);

		m_maxMinValuesPrTs[timeStepIndex].first = min;
		m_maxMinValuesPrTs[timeStepIndex].second = max;
	}

	min = m_maxMinValuesPrTs[timeStepIndex].first;
	max = m_maxMinValuesPrTs[timeStepIndex].second;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStatisticsDataCache::posNegClosestToZero(double& pos, double& neg)
{
	if (m_posClosestToZero == HUGE_VAL)
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

		m_posClosestToZero = pos;
		m_negClosestToZero = neg;
	}

	pos = m_posClosestToZero;
	neg = m_negClosestToZero;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStatisticsDataCache::posNegClosestToZero(size_t timeStepIndex, double& pos, double& neg)
{
	if (timeStepIndex >= m_posNegClosestToZeroPrTs.size())
	{
		m_posNegClosestToZeroPrTs.resize(timeStepIndex + 1, std::make_pair(HUGE_VAL, -HUGE_VAL));
	}

	if (m_posNegClosestToZeroPrTs[timeStepIndex].first == HUGE_VAL)
	{
        pos = HUGE_VAL;
        neg = -HUGE_VAL;
        
        m_statisticsCalculator->posNegClosestToZero(timeStepIndex, pos, neg);

		m_posNegClosestToZeroPrTs[timeStepIndex].first = pos;
		m_posNegClosestToZeroPrTs[timeStepIndex].second = neg;
	}

	pos = m_posNegClosestToZeroPrTs[timeStepIndex].first;
	neg = m_posNegClosestToZeroPrTs[timeStepIndex].second;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RigStatisticsDataCache::cellScalarValuesHistogram()
{
	if (m_histogram.size() == 0)
	{
		double min;
		double max;
		size_t nBins = 100;
		this->minMaxCellScalarValues(min, max);

		RigHistogramCalculator histCalc(min, max, nBins, &m_histogram);

		m_statisticsCalculator->addDataToHistogramCalculator(histCalc);

		m_p10 = histCalc.calculatePercentil(0.1);
		m_p90 = histCalc.calculatePercentil(0.9);
	}

	return m_histogram;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStatisticsDataCache::p10p90CellScalarValues(double& p10, double& p90)
{
	// First make sure they are calculated
	const std::vector<size_t>& histogr = this->cellScalarValuesHistogram();

	p10 = m_p10;
	p90 = m_p90;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStatisticsDataCache::meanCellScalarValues(double& meanValue)
{
	if (m_meanValue == HUGE_VAL)
	{
		m_statisticsCalculator->meanCellScalarValue(m_meanValue);
	}

	meanValue = m_meanValue;
}

