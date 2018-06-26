/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RimDerivedEnsembleCase.h"

#include "RiaSummaryTools.h"
#include "RiaTimeHistoryCurveMerger.h"

#include "RifDerivedEnsembleReader.h"

#include "RimDerivedEnsembleCaseCollection.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryPlotCollection.h"
#include "RimSummaryCaseCollection.h"

#include "cvfAssert.h"

#include <QFileInfo>

CAF_PDM_ABSTRACT_SOURCE_INIT(RimDerivedEnsembleCase, "RimDerivedEnsembleCase");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<time_t> RimDerivedEnsembleCase::EMPTY_TIME_STEPS_VECTOR;
const std::vector<double> RimDerivedEnsembleCase::EMPTY_VALUES_VECTOR;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimDerivedEnsembleCase::RimDerivedEnsembleCase() : m_summaryCase1(nullptr), m_summaryCase2(nullptr)
{
    CAF_PDM_InitObject("Summary Case",":/SummaryCase16x16.png","","");
    CAF_PDM_InitFieldNoDefault(&m_summaryCase1, "SummaryCase1", "SummaryCase1", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_summaryCase2, "SummaryCase2", "SummaryCase2", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimDerivedEnsembleCase::~RimDerivedEnsembleCase()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimDerivedEnsembleCase::setInUse(bool inUse)
{
    m_inUse = inUse;

    if (!m_inUse)
    {
        m_summaryCase1 = nullptr;
        m_summaryCase2 = nullptr;
        m_data.clear();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimDerivedEnsembleCase::isInUse() const
{
    return m_inUse;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimDerivedEnsembleCase::setSummaryCases(RimSummaryCase* sumCase1, RimSummaryCase* sumCase2)
{
    if (!sumCase1 || !sumCase2) return;
    m_summaryCase1 = sumCase1;
    m_summaryCase2 = sumCase2;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimDerivedEnsembleCase::needsCalculation(const RifEclipseSummaryAddress& address) const
{
    return m_data.count(address) == 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RimDerivedEnsembleCase::timeSteps(const RifEclipseSummaryAddress& address) const
{
    if (m_data.count(address) == 0) return EMPTY_TIME_STEPS_VECTOR;
    return m_data.at(address).first;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RimDerivedEnsembleCase::values(const RifEclipseSummaryAddress& address) const
{
    if (m_data.count(address) == 0) return EMPTY_VALUES_VECTOR;
    return m_data.at(address).second;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimDerivedEnsembleCase::calculate(const RifEclipseSummaryAddress& address)
{
    clearData(address);

    RifSummaryReaderInterface*  reader1 = m_summaryCase1 ? m_summaryCase1->summaryReader() : nullptr;
    RifSummaryReaderInterface*  reader2 = m_summaryCase2 ? m_summaryCase2->summaryReader() : nullptr;
    if (!reader1 || !reader2 || !parentEnsemble()) return;

    RiaTimeHistoryCurveMerger   merger;
    std::vector<double>         values1;
    std::vector<double>         values2;
    DerivedEnsembleOperator     op = parentEnsemble()->op();

    reader1->values(address, &values1);
    reader2->values(address, &values2);

    merger.addCurveData(values1, reader1->timeSteps(address));
    merger.addCurveData(values2, reader2->timeSteps(address));
    merger.computeInterpolatedValues();

    size_t sampleCount = merger.allTimeSteps().size();
    std::vector<double> calculatedValues;
    calculatedValues.reserve(sampleCount);
    for (size_t i = 0; i < sampleCount; i++)
    {
        if (op == DERIVED_ENSEMBLE_SUB)
        {
            calculatedValues.push_back(values1[i] - values2[i]);
        }
        else if (op == DERIVED_ENSEMBLE_ADD)
        {
            calculatedValues.push_back(values1[i] + values2[i]);
        }
    }

    auto& dataItem = m_data[address];
    dataItem.first = merger.allTimeSteps();
    dataItem.second = calculatedValues;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimDerivedEnsembleCase::caseName()
{
    auto case1Name = m_summaryCase1->caseName();
    auto case2Name = m_summaryCase2->caseName();

    if (case1Name == case2Name) return case1Name;
    else                        return QString("%1/%2").arg(case1Name).arg(case2Name);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimDerivedEnsembleCase::createSummaryReaderInterface()
{
    m_reader.reset(new RifDerivedEnsembleReader(this));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimDerivedEnsembleCase::summaryReader()
{
    return m_reader.get();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimDerivedEnsembleCase::updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath)
{
    // NOP
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimDerivedEnsembleCaseCollection * RimDerivedEnsembleCase::parentEnsemble() const
{
    RimDerivedEnsembleCaseCollection* ensemble;
    firstAncestorOrThisOfType(ensemble);
    return ensemble;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<time_t>, std::vector<double>> RimDerivedEnsembleCase::lookupCachedData(const RifEclipseSummaryAddress& address)
{
    auto itr = m_data.find(address);
    if (itr == m_data.end()) return std::make_pair(std::vector<time_t>(), std::vector<double>());
    return itr->second;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimDerivedEnsembleCase::clearData(const RifEclipseSummaryAddress& address)
{
    m_data.erase(address);
}
