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
/// Internal constants
//--------------------------------------------------------------------------------------------------
#define DOUBLE_INF  std::numeric_limits<double>::infinity()

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
std::vector<time_t> RiaTimeHistoryCurveResampler::timeStepsFromTimeRange(DateTimePeriod period, time_t minTime, time_t maxTime)
{
    if(minTime > maxTime) return std::vector<time_t>();

    auto firstOriginalTimeStep = QDT::fromTime_t(minTime);
    auto lastOriginalTimeStep = QDT::fromTime_t(maxTime);

    auto currTimeStep = firstResampledTimeStep(firstOriginalTimeStep, period);

    std::vector<time_t> timeSteps;
    while (QDT::lessThan(currTimeStep, lastOriginalTimeStep))
    {
        timeSteps.push_back(currTimeStep.toTime_t());
        currTimeStep = QDT::addPeriod(currTimeStep, period);
    }
    timeSteps.push_back(currTimeStep.toTime_t());

    return timeSteps;
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
        time_t periodStart = i > 0 ? m_timeSteps[i - 1] :
            QDT::subtractPeriod(QDT::fromTime_t(m_timeSteps[0]), period).toTime_t();
        time_t periodEnd = m_timeSteps[i];
        time_t periodLength = periodEnd - periodStart;

        while(true)
        {
            time_t origTimeStep = 0;
            double origValue = 0.0;

            if (oi > origDataSize) break;

            if (oi < origDataSize)
            {
                origTimeStep = origTimeSteps[oi];
                origValue = origValues[oi] != DOUBLE_INF ? origValues[oi] : 0.0;
            }
            else
            {
                origTimeStep = periodEnd;
                origValue = 0.0;
            }
            
            if (oi == 0)
            {
                if (origTimeStep == m_timeSteps[i])
                {
                    wMean += origValue;
                    oi++;
                    break;
                }
                origValue = 0.0;
            }

            time_t startTime = oi > 0 ? std::max(origTimeSteps[oi - 1], periodStart) : periodStart;
            time_t endTime = std::min(origTimeStep, periodEnd);
            
            wMean += origValue * (endTime - startTime) / periodLength;

            if (origTimeStep > m_timeSteps[i]) break;
            if (origTimeStep == m_timeSteps[i])
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
    size_t oi = 0;
    auto& origTimeSteps = m_originalValues.second;
    auto& origValues = m_originalValues.first;

    computeResampledTimeSteps(period);

    m_values.reserve(m_timeSteps.size());
    for (size_t i = 0; i < m_timeSteps.size(); i++)
    {
        while (oi < origDataSize && origTimeSteps[oi] < m_timeSteps[i]) oi++;

        time_t origTimeStep = oi < origDataSize ? origTimeSteps[oi] : m_timeSteps[i];
        double origValue = oi < origDataSize ? origValues[oi] : origValues[oi - 1];
        
        double value;
        if (oi > 0 && origTimeStep >= m_timeSteps[i])
        {
            value = interpolatedValue(m_timeSteps[i], origTimeSteps[oi - 1], origValues[oi - 1], origTimeStep, origValue);
        }
        else
        {
            value = origValue;
        }

        m_values.push_back(value);
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
    CVF_ASSERT(period != DateTimePeriod::NONE && m_originalValues.second.size() > 0);

    auto firstOriginalTimeStep = QDT::fromTime_t(m_originalValues.second.front());
    auto lastOriginalTimeStep = QDT::fromTime_t(m_originalValues.second.back());

    clearData();
    auto currTimeStep = firstResampledTimeStep(firstOriginalTimeStep, period);

    while (QDT::lessThan(currTimeStep, lastOriginalTimeStep))
    {
        m_timeSteps.push_back(currTimeStep.toTime_t());
        currTimeStep = QDT::addPeriod(currTimeStep, period);
    }

    // Add last time step
    m_timeSteps.push_back(currTimeStep.toTime_t());
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RiaTimeHistoryCurveResampler::interpolatedValue(time_t t, time_t t1, double v1, time_t t2, double v2)
{
    CVF_ASSERT(t2 >= t1);

    if (t <= t1) return v1;
    if (t >= t2) return v2;

    return (v2 - v1) * (double)(t - t1) / (double)(t2 - t1) + v1;
}
