/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include <cvfConfigCore.h>
#include <cvfAssert.h>

#include "RiaTimeHistoryCurveResampler.h"

//QString tostring(const QDateTime& dt)
//{
//    int y = dt.date().year();
//    int m = dt.date().month();
//    int d = dt.date().day();
//
//    int h = dt.time().hour();
//    int mm = dt.time().minute();
//    int s = dt.time().second();
//
//    return QString("%1.%2.%3 %4:%5:%6").arg(y).arg(m).arg(d).arg(h).arg(mm).arg(s);
//}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaTimeHistoryCurveResampler::RiaTimeHistoryCurveResampler()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaTimeHistoryCurveResampler::setCurveData(const std::vector<double>& values, const std::vector<time_t>& timeSteps)
{
    CVF_ASSERT(values.size() == timeSteps.size());

    clearData();
    m_originalValues = std::make_pair(values, timeSteps);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaTimeHistoryCurveResampler::resampleAndComputePeriodEndValues(DateTimePeriod period)
{
    computePeriodEndValues(period);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaTimeHistoryCurveResampler::resampleAndComputeWeightedMeanValues(DateTimePeriod period)
{
    computeWeightedMeanValues(period);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RiaTimeHistoryCurveResampler::resampledTimeSteps() const
{
    return m_timeSteps;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RiaTimeHistoryCurveResampler::resampledValues() const
{
    return m_values;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaTimeHistoryCurveResampler::computeWeightedMeanValues(DateTimePeriod period)
{
    size_t origDataSize = m_originalValues.second.size();
    size_t oi = 0;
    auto& origTimeSteps = m_originalValues.second;
    auto& origValues = m_originalValues.first;

    computeResampledTimeSteps(period);

    m_values.reserve(m_timeSteps.size());
    for (size_t i = 0; i < m_timeSteps.size(); i++)
    {
        double wMean = 0.0;
        time_t periodStart = i > 0 ? m_timeSteps[i - 1] : origTimeSteps[0];
        time_t periodEnd = m_timeSteps[i];
        time_t periodLength = periodEnd - periodStart;

        while(true)
        {
            if (oi == origDataSize) break;

            if (oi == 0)
            {
                if (origTimeSteps[oi] == m_timeSteps[i])
                {
                    wMean += origValues[0];
                    oi++;
                    break;
                }
                oi++;
                continue;
            }

            time_t startTime = std::max(origTimeSteps[oi-1], periodStart);
            time_t endTime = std::min(origTimeSteps[oi], periodEnd);
            
            wMean += origValues[oi] * (endTime - startTime) / periodLength;

            if (origTimeSteps[oi] > m_timeSteps[i]) break;
            if (origTimeSteps[oi] == m_timeSteps[i])
            {
                oi++;
                break;
            }
            oi++;
        }

        m_values.push_back(wMean);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaTimeHistoryCurveResampler::computePeriodEndValues(DateTimePeriod period)
{
    size_t origDataSize = m_originalValues.second.size();
    size_t origIndex = 0;
    auto& origTimeSteps = m_originalValues.second;
    auto& origValues = m_originalValues.first;

    computeResampledTimeSteps(period);

    m_values.reserve(m_timeSteps.size());
    for (size_t i = 0; i < m_timeSteps.size(); i++)
    {
        while (origIndex < origDataSize && origTimeSteps[origIndex] < m_timeSteps[i]) origIndex++;
        m_values.push_back(origValues[origIndex]);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaTimeHistoryCurveResampler::clearData()
{
    m_timeSteps.clear();
    m_values.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaTimeHistoryCurveResampler::computeResampledTimeSteps(DateTimePeriod period)
{
    CVF_ASSERT(m_originalValues.second.size() > 0);

    auto firstOriginalTimeStep = QDT::fromTime_t(m_originalValues.second.front());
    auto lastOriginalTimeStep = QDT::fromTime_t(m_originalValues.second.back());

    clearData();
    auto currTimeStep = firstResampledTimeStep(firstOriginalTimeStep, period);
    while (QDT::lessThanOrEqualTo(currTimeStep, lastOriginalTimeStep))
    {
        auto ss1 = currTimeStep.toString();

        m_timeSteps.push_back(currTimeStep.toTime_t());
        currTimeStep = QDT::addPeriod(currTimeStep, period);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime RiaTimeHistoryCurveResampler::firstResampledTimeStep(const QDateTime& firstTimeStep, DateTimePeriod period)
{
    QDateTime truncatedTime = QDT::truncateTime(firstTimeStep, period);

    if (QDT::lessThan(truncatedTime, firstTimeStep)) return QDT::addPeriod(truncatedTime, period);
    return truncatedTime;
}
