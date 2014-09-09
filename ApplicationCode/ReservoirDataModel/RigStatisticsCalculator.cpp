/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RigStatisticsMath.h"
#include "RigCaseCellResultsData.h"

#include <cmath> // Needed for HUGE_VAL on Linux


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStatisticsCalculator::meanCellScalarValue(double& meanValue)
{
    double valueSum = 0.0;
    size_t sampleCount = 0;

    this->valueSumAndSampleCount(valueSum, sampleCount);

    if (sampleCount == 0)
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
RigNativeStatCalc::RigNativeStatCalc(RigCaseCellResultsData* cellResultsData, size_t scalarResultIndex)
	: m_resultsData(cellResultsData),
    m_scalarResultIndex(scalarResultIndex)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigNativeStatCalc::minMaxCellScalarValues(size_t timeStepIndex, double& min, double& max)
{
	std::vector<double>& values = m_resultsData->cellScalarResults(m_scalarResultIndex, timeStepIndex);

	size_t i;
	for (i = 0; i < values.size(); i++)
	{
		if (values[i] == HUGE_VAL)
		{
			continue;
		}

		if (values[i] < min)
		{
			min = values[i];
		}

		if (values[i] > max)
		{
			max = values[i];
		}
	}
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigNativeStatCalc::posNegClosestToZero(size_t timeStepIndex, double& pos, double& neg)
{
    std::vector<double>& values = m_resultsData->cellScalarResults(m_scalarResultIndex, timeStepIndex);

	size_t i;
	for (i = 0; i < values.size(); i++)
	{
		if (values[i] == HUGE_VAL)
		{
			continue;
		}

		if (values[i] < pos && values[i] > 0)
		{
			pos = values[i];
		}

		if (values[i] > neg && values[i] < 0)
		{
			neg = values[i];
		}
	}
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigNativeStatCalc::addDataToHistogramCalculator(RigHistogramCalculator& histogramCalculator)
{
	for (size_t tIdx = 0; tIdx < m_resultsData->timeStepCount(m_scalarResultIndex); tIdx++)
	{
        std::vector<double>& values = m_resultsData->cellScalarResults(m_scalarResultIndex, tIdx);

		histogramCalculator.addData(values);
	}
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigNativeStatCalc::valueSumAndSampleCount(double& valueSum, size_t& sampleCount)
{
    for (size_t tIdx = 0; tIdx < m_resultsData->timeStepCount(m_scalarResultIndex); tIdx++)
    {
        std::vector<double>& values = m_resultsData->cellScalarResults(m_scalarResultIndex, tIdx);
        size_t undefValueCount = 0;
        for (size_t cIdx = 0; cIdx < values.size(); ++cIdx)
        {
            double value = values[cIdx];
            if (value == HUGE_VAL || value != value)
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
size_t RigNativeStatCalc::timeStepCount()
{
    return m_resultsData->timeStepCount(m_scalarResultIndex);
}





//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigMultipleDatasetStatCalc::RigMultipleDatasetStatCalc()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigMultipleDatasetStatCalc::addStatisticsCalculator(RigStatisticsCalculator* statisticsCalculator)
{
    if (statisticsCalculator)
    {
        m_nativeStatisticsCalculators.push_back(statisticsCalculator);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigMultipleDatasetStatCalc::minMaxCellScalarValues(size_t timeStepIndex, double& min, double& max)
{
    for (size_t i = 0; i < m_nativeStatisticsCalculators.size(); i++)
    {
        if (m_nativeStatisticsCalculators.at(i))
        {
            m_nativeStatisticsCalculators.at(i)->minMaxCellScalarValues(timeStepIndex, min, max);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigMultipleDatasetStatCalc::posNegClosestToZero(size_t timeStepIndex, double& pos, double& neg)
{
    for (size_t i = 0; i < m_nativeStatisticsCalculators.size(); i++)
    {
        if (m_nativeStatisticsCalculators.at(i))
        {
            m_nativeStatisticsCalculators.at(i)->posNegClosestToZero(timeStepIndex, pos, neg);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigMultipleDatasetStatCalc::valueSumAndSampleCount(double& valueSum, size_t& sampleCount)
{
    for (size_t i = 0; i < m_nativeStatisticsCalculators.size(); i++)
    {
        if (m_nativeStatisticsCalculators.at(i))
        {
            m_nativeStatisticsCalculators.at(i)->valueSumAndSampleCount(valueSum, sampleCount);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigMultipleDatasetStatCalc::addDataToHistogramCalculator(RigHistogramCalculator& histogramCalculator)
{
    for (size_t i = 0; i < m_nativeStatisticsCalculators.size(); i++)
    {
        if (m_nativeStatisticsCalculators.at(i))
        {
            m_nativeStatisticsCalculators.at(i)->addDataToHistogramCalculator(histogramCalculator);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigMultipleDatasetStatCalc::timeStepCount()
{
    if (m_nativeStatisticsCalculators.size() > 0)
    {
        return m_nativeStatisticsCalculators[0]->timeStepCount();
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigMultipleDatasetStatCalc::addNativeStatisticsCalculator(RigCaseCellResultsData* cellResultsData, size_t scalarResultIndex)
{
    if (scalarResultIndex != cvf::UNDEFINED_SIZE_T)
    {
        this->addStatisticsCalculator(new RigNativeStatCalc(cellResultsData, scalarResultIndex));
    }
}

