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

#include "RigStatisticsMath.h"
#include <algorithm>
#include <assert.h>
#include <math.h>
#include "cvfBase.h"
#include "cvfMath.h"

//--------------------------------------------------------------------------------------------------
/// A function to do basic statistical calculations 
//--------------------------------------------------------------------------------------------------

void RigStatisticsMath::calculateBasicStatistics(const std::vector<double>& values, double* min, double* max, double* sum, double* range, double* mean, double* dev)
{
    double m_min(HUGE_VAL);
    double m_max(-HUGE_VAL);
    double m_mean(HUGE_VAL);
    double m_dev(HUGE_VAL);

    double m_sum = 0.0;
    double sumSquared = 0.0;

    size_t validValueCount = 0;

    for (size_t i = 0; i < values.size(); i++)
    {
        double val = values[i];
        if (RiaStatisticsTools::isInvalidNumber<double>(val)) continue;

        validValueCount++;

        if (val < m_min) m_min = val;
        if (val > m_max) m_max = val;

        m_sum += val;
        sumSquared += (val * val);
    }

    if (validValueCount > 0)
    {
        m_mean = m_sum / validValueCount;

        // http://en.wikipedia.org/wiki/Standard_deviation#Rapid_calculation_methods
        // Running standard deviation

        double s0 = static_cast<double>(validValueCount);
        double s1 = m_sum;
        double s2 = sumSquared;

        m_dev = sqrt( (s0 * s2) - (s1 * s1) ) / s0;
    }

    if (min) *min = m_min;
    if (max) *max = m_max;
    if (sum) *sum = m_sum;
    if (range) *range = m_max - m_min;

    if (mean) *mean = m_mean;
    if (dev) *dev = m_dev;
}


//--------------------------------------------------------------------------------------------------
/// Calculate the percentiles of /a inputValues at the pValPosition percentages using the "Nearest Rank"
/// method. This method treats HUGE_VAL as "undefined" values, and ignores these. Will return HUGE_VAL if
/// the inputValues does not contain any valid values
//--------------------------------------------------------------------------------------------------

std::vector<double> RigStatisticsMath::calculateNearestRankPercentiles(const std::vector<double> & inputValues, const std::vector<double>& pValPositions)
{
    std::vector<double> sortedValues;
    sortedValues.reserve(inputValues.size());

    for (size_t i = 0; i < inputValues.size(); ++i)
    {
        if (RiaStatisticsTools::isValidNumber<double>(inputValues[i]))
        {
            sortedValues.push_back(inputValues[i]);
        }
    }

    std::sort(sortedValues.begin(), sortedValues.end());

    std::vector<double> percentiles(pValPositions.size(), HUGE_VAL);
    if (sortedValues.size())
    {
        for (size_t i = 0; i < pValPositions.size(); ++i)
        {
            double pVal = HUGE_VAL;

            size_t pValIndex = static_cast<size_t>(sortedValues.size() * cvf::Math::abs(pValPositions[i]) / 100);

            if (pValIndex >= sortedValues.size() ) pValIndex = sortedValues.size() - 1;

            pVal = sortedValues[pValIndex];
            percentiles[i] = pVal;
        }
    }

    return percentiles;
};

//--------------------------------------------------------------------------------------------------
/// Calculate the percentiles of /a inputValues at the pValPosition percentages by interpolating input values.
/// This method treats HUGE_VAL as "undefined" values, and ignores these. Will return HUGE_VAL if
/// the inputValues does not contain any valid values
//--------------------------------------------------------------------------------------------------
std::vector<double> RigStatisticsMath::calculateInterpolatedPercentiles(const std::vector<double> & inputValues, const std::vector<double>& pValPositions)
{
    std::vector<double> sortedValues;
    sortedValues.reserve(inputValues.size());

    for (size_t i = 0; i < inputValues.size(); ++i)
    {
        if (RiaStatisticsTools::isValidNumber<double>(inputValues[i]))
        {
            sortedValues.push_back(inputValues[i]);
        }
    }

    std::sort(sortedValues.begin(), sortedValues.end());

    std::vector<double> percentiles(pValPositions.size(), HUGE_VAL);
    if (sortedValues.size())
    {
        for (size_t i = 0; i < pValPositions.size(); ++i)
        {
            double pVal = HUGE_VAL;

            double doubleIndex = (sortedValues.size() - 1) * cvf::Math::abs(pValPositions[i]) / 100.0;

            size_t lowerValueIndex = static_cast<size_t>(floor(doubleIndex));
            size_t upperValueIndex = lowerValueIndex + 1;

            double upperValueWeight = doubleIndex - lowerValueIndex;
            assert(upperValueWeight < 1.0);

            if (upperValueIndex < sortedValues.size())
            {
                pVal = (1.0 - upperValueWeight) * sortedValues[lowerValueIndex] + upperValueWeight * sortedValues[upperValueIndex];
            }
            else
            {
                pVal = sortedValues[lowerValueIndex];
            }
            percentiles[i] = pVal;
        }
    }

    return percentiles;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigHistogramCalculator::RigHistogramCalculator(double min, double max, size_t nBins, std::vector<size_t>* histogram)
{
    assert(histogram);
    assert(nBins > 0);

    if (max == min) {  nBins = 1; } // Avoid dividing on 0 range

    m_histogram = histogram;
    m_min = min;
    m_observationCount = 0;

    // Initialize bins
    m_histogram->resize(nBins);
    for (size_t i = 0; i < m_histogram->size(); ++i) (*m_histogram)[i] = 0;

    m_range = max - min;
    m_maxIndex = nBins-1;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigHistogramCalculator::addValue(double value)
{
    if (RiaStatisticsTools::isInvalidNumber<double>(value)) return;

    size_t index = 0;

    if (m_maxIndex > 0) index = (size_t)(m_maxIndex*(value - m_min)/m_range);

    if (index < m_histogram->size()) // Just clip to the max min range (-index will overflow to positive )
    {
        (*m_histogram)[index]++;
        m_observationCount++;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigHistogramCalculator::addData(const std::vector<double>& data)
{
    assert(m_histogram);
    for (size_t i = 0; i < data.size(); ++i) 
    {
        addValue(data[i]);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigHistogramCalculator::addData(const std::vector<float>& data)
{
    assert(m_histogram);
    for (size_t i = 0; i < data.size(); ++i) 
    {
        addValue(data[i]);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigHistogramCalculator::calculatePercentil(double pVal)
{
    assert(m_histogram);
    assert(m_histogram->size());
    assert( 0.0 <= pVal && pVal <= 1.0);

    double pValObservationCount = pVal*m_observationCount;
    if (pValObservationCount == 0.0) return m_min;

    size_t accObsCount = 0;
    double binWidth =  m_range/m_histogram->size();
    for (size_t binIdx = 0; binIdx < m_histogram->size(); ++binIdx)
    {
        size_t binObsCount = (*m_histogram)[binIdx];

        accObsCount += binObsCount;
        if (accObsCount >= pValObservationCount)
        {
            double domainValueAtEndOfBin = m_min + (binIdx+1) * binWidth;
            double unusedFractionOfLastBin = (double)(accObsCount - pValObservationCount)/binObsCount;
            return domainValueAtEndOfBin - unusedFractionOfLastBin*binWidth;
        }
    }
    assert(false);

    return HUGE_VAL;
}
