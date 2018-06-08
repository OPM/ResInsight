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

#include "RimEnsembleCurveSet.h"


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
void RimEnsembleStatisticsCase::calculate(const RimSummaryCaseCollection* ensemble, const RifEclipseSummaryAddress& inputAddress)
{
    RiaTimeHistoryCurveMerger curveMerger;
    int caseCount = (int)ensemble->allSummaryCases().size();

    {
        for (const auto& sumCase : ensemble->allSummaryCases())
        {
            const auto& reader = sumCase->summaryReader();
            if (reader)
            {
                std::vector<time_t> timeSteps = reader->timeSteps(inputAddress);
                std::vector<double> values;
                reader->values(inputAddress, &values);

                curveMerger.addCurveData(values, timeSteps);
            }
        }
        curveMerger.computeInterpolatedValues();
    }

    const std::vector<time_t>& allTimeSteps = curveMerger.allTimeSteps();
    std::vector<std::vector<double>> allValues;
    {
        for (int c = 0; c < caseCount; c++)
        {
            allValues.push_back(curveMerger.interpolatedCurveValuesForAllTimeSteps(c));
        }
    }

    clearData();
    m_timeSteps = allTimeSteps;

    for (int t = 0; t < (int)allTimeSteps.size(); t++)
    {
        std::vector<double> valuesForTimeSteps;
        valuesForTimeSteps.reserve(caseCount);
        
        for (int c = 0; c < caseCount; c++)
        {
            valuesForTimeSteps.push_back(allValues[c][t]);
        }

        {
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

    }
    m_addressUsedInLastCalculation = inputAddress;
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
        calculate(ensemble, inputAddress);
    }
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
