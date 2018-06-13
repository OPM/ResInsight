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

#include <limits>

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
        std::vector<double> valuesForTimeSteps;
        valuesForTimeSteps.reserve(sumCases.size());
        
        for (int c = 0; c < sumCases.size(); c++)
        {
            valuesForTimeSteps.push_back(allValues[c][t]);
        }

        double min, max, range, mean, stdev;
        RigStatisticsMath::calculateBasicStatistics(valuesForTimeSteps, &min, &max, nullptr, &range, &mean, &stdev);

        std::vector<size_t> histogram;
        RigHistogramCalculator histCalc(min, max, 100, &histogram);
        histCalc.addData(valuesForTimeSteps);

        double p10, p50, p90;
        p10 = histCalc.calculatePercentil(0.1);
        p50 = histCalc.calculatePercentil(0.5);
        p90 = histCalc.calculatePercentil(0.9);

        m_p10Data.push_back(p10);
        m_p50Data.push_back(p50);
        m_p90Data.push_back(p90);
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
