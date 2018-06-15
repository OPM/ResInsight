/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RimEnsembleStatisticsCase.h"

#include "RifEnsembleStatisticsReader.h"

#include "RigStatisticsMath.h"
#include "RiaTimeHistoryCurveMerger.h"
#include "RiaTimeHistoryCurveResampler.h"

#include "RimEnsembleCurveSet.h"

#include <vector>
#include <set>
#include <limits>

//--------------------------------------------------------------------------------------------------
/// Internal constants
//--------------------------------------------------------------------------------------------------
#define DOUBLE_INF  std::numeric_limits<double>::infinity()

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEnsembleStatisticsCase::RimEnsembleStatisticsCase(RimEnsembleCurveSet* curveSet)
{
    m_curveSet = curveSet;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RimEnsembleStatisticsCase::timeSteps() const
{
    return m_timeSteps;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RimEnsembleStatisticsCase::p10() const
{
    return m_p10Data;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RimEnsembleStatisticsCase::p50() const
{
    return m_p50Data;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RimEnsembleStatisticsCase::p90() const
{
    return m_p90Data;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RimEnsembleStatisticsCase::mean() const
{
    return m_meanData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimEnsembleStatisticsCase::caseName()
{
    return "Ensemble Statistics";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatisticsCase::createSummaryReaderInterface()
{
    m_statisticsReader.reset(new RifEnsembleStatisticsReader(this));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimEnsembleStatisticsCase::summaryReader()
{
    return m_statisticsReader.get();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RimEnsembleCurveSet* RimEnsembleStatisticsCase::curveSet() const
{
    return m_curveSet;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatisticsCase::calculate()
{
    auto inputAddress = m_curveSet->summaryAddress();
    auto ensemble = m_curveSet->summaryCaseCollection();
    if (m_statisticsReader && ensemble && inputAddress.isValid())
    {
        calculate(validSummaryCases(ensemble->allSummaryCases(), inputAddress), inputAddress);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatisticsCase::calculate(const std::vector<RimSummaryCase*> sumCases, const RifEclipseSummaryAddress& inputAddress)
{
    std::vector<time_t> allTimeSteps;
    std::vector<std::vector<double>> allValues;

    if (!inputAddress.isValid()) return;

    allValues.reserve(sumCases.size());
    for (const auto& sumCase : sumCases)
    {
        const auto& reader = sumCase->summaryReader();
        if (reader)
        {
            std::vector<time_t> timeSteps = reader->timeSteps(inputAddress);
            std::vector<double> values;
            reader->values(inputAddress, &values);

            RiaTimeHistoryCurveResampler resampler;
            resampler.setCurveData(values, timeSteps);
            if (inputAddress.hasAccumulatedData()) resampler.resampleAndComputePeriodEndValues(DateTimePeriod::DAY);
            else                                   resampler.resampleAndComputeWeightedMeanValues(DateTimePeriod::DAY);

            if (allTimeSteps.empty()) allTimeSteps = resampler.resampledTimeSteps();
            allValues.push_back(std::vector<double>(resampler.resampledValues().begin(), resampler.resampledValues().end()));
        }
    }

    clearData();
    m_timeSteps = allTimeSteps;

    for (int t = 0; t < (int)allTimeSteps.size(); t++)
    {
        std::vector<double> valuesAtTimeStep;
        valuesAtTimeStep.reserve(sumCases.size());
        
        for (int c = 0; c < (int)sumCases.size(); c++)
        {
            valuesAtTimeStep.push_back(allValues[c][t]);
        }

        double p10, p50, p90, mean;
        calculateStatistics(valuesAtTimeStep, &p10, &p50, &p90, &mean);

        if (p10 != DOUBLE_INF) m_p10Data.push_back(p10);
        if (p50 != DOUBLE_INF) m_p50Data.push_back(p50);
        if (p90 != DOUBLE_INF) m_p90Data.push_back(p90);
        m_meanData.push_back(mean);
    }
    m_addressUsedInLastCalculation = inputAddress;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatisticsCase::clearData()
{
    m_timeSteps.clear();
    m_p10Data.clear();
    m_p50Data.clear();
    m_p90Data.clear();
    m_meanData.clear();
    m_addressUsedInLastCalculation = RifEclipseSummaryAddress();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimEnsembleStatisticsCase::validSummaryCases(const std::vector<RimSummaryCase*> allSumCases, const RifEclipseSummaryAddress& inputAddress)
{
    std::vector<RimSummaryCase*> validCases;
    std::vector<std::tuple<RimSummaryCase*, time_t, time_t>> times;

    time_t minTimeStep = std::numeric_limits<time_t>::max();
    time_t maxTimeStep = 0;

    for (auto& sumCase : allSumCases)
    {
        const auto& reader = sumCase->summaryReader();
        if (reader)
        {
            std::vector<time_t> timeSteps = reader->timeSteps(inputAddress);
            if (!timeSteps.empty())
            {
                time_t firstTimeStep = timeSteps.front();
                time_t lastTimeStep = timeSteps.back();

                if (firstTimeStep < minTimeStep)    minTimeStep = firstTimeStep;
                if (lastTimeStep > maxTimeStep)      maxTimeStep = lastTimeStep;
                times.push_back(std::make_tuple(sumCase, firstTimeStep, lastTimeStep));
            }
        }
    }

    for (auto& item : times)
    {
        RimSummaryCase* sumCase = std::get<0>(item);
        time_t firstTimeStep = std::get<1>(item);
        time_t lastTimeStep = std::get<2>(item);

        if (firstTimeStep == minTimeStep && lastTimeStep == maxTimeStep)
        {
            validCases.push_back(sumCase);
        }
    }
    return validCases;
}

//--------------------------------------------------------------------------------------------------
/// Algorithm:
/// https://en.wikipedia.org/wiki/Percentile#Third_variant,_'%22%60UNIQ--postMath-00000052-QINU%60%22'
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatisticsCase::calculateStatistics(const std::vector<double>& values, double* p10, double* p50, double* p90, double* mean)
{
    CVF_ASSERT(p10 && p50 && p90 && mean);

    enum PValue { P10, P50, P90 };

    std::vector<double> sortedValues;
    double valueSum = 0;

    {
        std::multiset<double> vSet(values.begin(), values.end());
        for (double v : vSet)
        {
            sortedValues.push_back(v);
            valueSum += v;
        }
    }

    int valueCount = (int)sortedValues.size();
    double percentiles[] = { 0.1, 0.5, 0.9 };
    double pValues[] = { DOUBLE_INF, DOUBLE_INF, DOUBLE_INF };

    for (int i = P10; i <= P90; i++)
    {
        // Check valid params
        if ((percentiles[i] < 1.0 / ((double)valueCount + 1)) || (percentiles[i] > (double)valueCount / ((double)valueCount + 1))) continue;

        double rank = percentiles[i] * (valueCount + 1) - 1;
        double rankInt;
        double rankFrac = std::modf(rank, &rankInt);

        if (rankInt < valueCount - 1)
        {
            pValues[i] = sortedValues[rankInt] + rankFrac * (sortedValues[rankInt + 1] - sortedValues[rankInt]);
        }
        else
        {
            pValues[i] = sortedValues[rankInt];
        }
    }

    *p10 = pValues[P10];
    *p50 = pValues[P50];
    *p90 = pValues[P90];
    *mean = valueSum / valueCount;
}

