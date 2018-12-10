/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RigCaseCellResultsData.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RigEclipseCaseData.h"
#include "RigEclipseMultiPropertyStatCalc.h"
#include "RigEclipseNativeStatCalc.h"
#include "RigEclipseResultInfo.h"
#include "RigMainGrid.h"
#include "RigStatisticsDataCache.h"
#include "RigStatisticsMath.h"

#include "RimCompletionCellIntersectionCalc.h"
#include "RimEclipseCase.h"
#include "RimProject.h"

#include "RifReaderEclipseOutput.h"

#include "cafProgressInfo.h"
#include "cvfGeometryTools.h"

#include <QDateTime>

#include <algorithm>
#include <math.h>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCaseCellResultsData::RigCaseCellResultsData(RigEclipseCaseData* ownerCaseData)
    : m_activeCellInfo(nullptr)
{
    CVF_ASSERT(ownerCaseData != nullptr);
    CVF_ASSERT(ownerCaseData->mainGrid() != nullptr);

    m_ownerCaseData = ownerCaseData;
    m_ownerMainGrid = ownerCaseData->mainGrid();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::setMainGrid(RigMainGrid* ownerGrid)
{
    m_ownerMainGrid = ownerGrid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::setActiveCellInfo(RigActiveCellInfo* activeCellInfo)
{
    m_activeCellInfo = activeCellInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::minMaxCellScalarValues(size_t scalarResultIndex, double& min, double& max)
{
    m_statisticsDataCache[scalarResultIndex]->minMaxCellScalarValues(min, max);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::minMaxCellScalarValues(size_t scalarResultIndex, size_t timeStepIndex, double& min, double& max)
{
    m_statisticsDataCache[scalarResultIndex]->minMaxCellScalarValues(timeStepIndex, min, max);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::posNegClosestToZero(size_t scalarResultIndex, double& pos, double& neg)
{
    m_statisticsDataCache[scalarResultIndex]->posNegClosestToZero(pos, neg);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::posNegClosestToZero(size_t scalarResultIndex, size_t timeStepIndex, double& pos, double& neg)
{
    m_statisticsDataCache[scalarResultIndex]->posNegClosestToZero(timeStepIndex, pos, neg);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RigCaseCellResultsData::cellScalarValuesHistogram(size_t scalarResultIndex)
{
    return m_statisticsDataCache[scalarResultIndex]->cellScalarValuesHistogram();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RigCaseCellResultsData::cellScalarValuesHistogram(size_t scalarResultIndex, size_t timeStepIndex)
{
    return m_statisticsDataCache[scalarResultIndex]->cellScalarValuesHistogram(timeStepIndex);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::p10p90CellScalarValues(size_t scalarResultIndex, double& p10, double& p90)
{
    m_statisticsDataCache[scalarResultIndex]->p10p90CellScalarValues(p10, p90);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::p10p90CellScalarValues(size_t scalarResultIndex, size_t timeStepIndex, double& p10, double& p90)
{
    m_statisticsDataCache[scalarResultIndex]->p10p90CellScalarValues(timeStepIndex, p10, p90);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::meanCellScalarValues(size_t scalarResultIndex, double& meanValue)
{
    m_statisticsDataCache[scalarResultIndex]->meanCellScalarValues(meanValue);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::meanCellScalarValues(size_t scalarResultIndex, size_t timeStepIndex, double& meanValue)
{
    m_statisticsDataCache[scalarResultIndex]->meanCellScalarValues(timeStepIndex, meanValue);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<int>& RigCaseCellResultsData::uniqueCellScalarValues(size_t scalarResultIndex)
{
    return m_statisticsDataCache[scalarResultIndex]->uniqueCellScalarValues();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::sumCellScalarValues(size_t scalarResultIndex, double& sumValue)
{
    m_statisticsDataCache[scalarResultIndex]->sumCellScalarValues(sumValue);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::sumCellScalarValues(size_t scalarResultIndex, size_t timeStepIndex, double& sumValue)
{
    m_statisticsDataCache[scalarResultIndex]->sumCellScalarValues(timeStepIndex, sumValue);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::mobileVolumeWeightedMean(size_t scalarResultIndex, double& meanValue)
{
    m_statisticsDataCache[scalarResultIndex]->mobileVolumeWeightedMean(meanValue);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::mobileVolumeWeightedMean(size_t scalarResultIndex, size_t timeStepIndex, double& meanValue)
{
    m_statisticsDataCache[scalarResultIndex]->mobileVolumeWeightedMean(timeStepIndex, meanValue);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::resultCount() const
{
    return m_cellScalarResults.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::timeStepCount(size_t scalarResultIndex) const
{
    CVF_TIGHT_ASSERT(scalarResultIndex < resultCount());

    return m_cellScalarResults[scalarResultIndex].size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<std::vector<double>>& RigCaseCellResultsData::cellScalarResults(size_t scalarResultIndex) const
{
    CVF_TIGHT_ASSERT(scalarResultIndex < resultCount());

    return m_cellScalarResults[scalarResultIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>>& RigCaseCellResultsData::cellScalarResults(size_t scalarResultIndex)
{
    CVF_TIGHT_ASSERT(scalarResultIndex < resultCount());

    return m_cellScalarResults[scalarResultIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double>& RigCaseCellResultsData::cellScalarResults(size_t scalarResultIndex, size_t timeStepIndex)
{
    CVF_TIGHT_ASSERT(scalarResultIndex < resultCount());
    CVF_TIGHT_ASSERT(timeStepIndex < m_cellScalarResults[scalarResultIndex].size());

    return m_cellScalarResults[scalarResultIndex][timeStepIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::findScalarResultIndex(RiaDefines::ResultCatType type, const QString& resultName) const
{
    std::vector<RigEclipseResultInfo>::const_iterator it;
    for (it = m_resultInfos.begin(); it != m_resultInfos.end(); ++it)
    {
        if (it->resultType() == type && it->resultName() == resultName)
        {
            return it->gridScalarResultIndex();
        }
    }

    return cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::findScalarResultIndex(const QString& resultName) const
{
    size_t scalarResultIndex = this->findScalarResultIndex(RiaDefines::STATIC_NATIVE, resultName);

    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        scalarResultIndex = this->findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, resultName);
    }

    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        scalarResultIndex = this->findScalarResultIndex(RiaDefines::SOURSIMRL, resultName);
    }

    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        scalarResultIndex = this->findScalarResultIndex(RiaDefines::GENERATED, resultName);
    }

    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        scalarResultIndex = this->findScalarResultIndex(RiaDefines::INPUT_PROPERTY, resultName);
    }

    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        scalarResultIndex = this->findScalarResultIndex(RiaDefines::FORMATION_NAMES, resultName);
    }

    return scalarResultIndex;
}

//--------------------------------------------------------------------------------------------------
/// Adds an empty scalar set, and returns the scalarResultIndex to it.
/// if resultName already exists, it just returns the scalarResultIndex to the existing result.
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::findOrCreateScalarResultIndex(RiaDefines::ResultCatType type,
                                                             const QString&            resultName,
                                                             bool                      needsToBeStored)
{
    size_t scalarResultIndex = this->findScalarResultIndex(type, resultName);

    // If the result exists, do nothing

    if (scalarResultIndex != cvf::UNDEFINED_SIZE_T)
    {
        return scalarResultIndex;
    }

    // Create the new empty result with metadata

    scalarResultIndex = this->resultCount();
    m_cellScalarResults.push_back(std::vector<std::vector<double>>());
    RigEclipseResultInfo resInfo(type, resultName, needsToBeStored, false, scalarResultIndex);
    m_resultInfos.push_back(resInfo);

    // Create statistics calculator and add statistics cache object
    // Todo: Move to a "factory" method

    cvf::ref<RigStatisticsCalculator> statisticsCalculator;

    if (resultName == RiaDefines::combinedTransmissibilityResultName())
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();

        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RiaDefines::STATIC_NATIVE, "TRANX"));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RiaDefines::STATIC_NATIVE, "TRANY"));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RiaDefines::STATIC_NATIVE, "TRANZ"));

        statisticsCalculator = calc;
    }
    else if (resultName == RiaDefines::combinedMultResultName())
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();

        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RiaDefines::STATIC_NATIVE, "MULTX"));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RiaDefines::STATIC_NATIVE, "MULTX-"));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RiaDefines::STATIC_NATIVE, "MULTY"));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RiaDefines::STATIC_NATIVE, "MULTY-"));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RiaDefines::STATIC_NATIVE, "MULTZ"));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RiaDefines::STATIC_NATIVE, "MULTZ-"));

        statisticsCalculator = calc;
    }
    else if (resultName == RiaDefines::combinedRiTranResultName())
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();
        calc->addNativeStatisticsCalculator(this,
                                            findScalarResultIndex(RiaDefines::STATIC_NATIVE, RiaDefines::riTranXResultName()));
        calc->addNativeStatisticsCalculator(this,
                                            findScalarResultIndex(RiaDefines::STATIC_NATIVE, RiaDefines::riTranYResultName()));
        calc->addNativeStatisticsCalculator(this,
                                            findScalarResultIndex(RiaDefines::STATIC_NATIVE, RiaDefines::riTranZResultName()));
        statisticsCalculator = calc;
    }
    else if (resultName == RiaDefines::combinedRiMultResultName())
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();
        calc->addNativeStatisticsCalculator(this,
                                            findScalarResultIndex(RiaDefines::STATIC_NATIVE, RiaDefines::riMultXResultName()));
        calc->addNativeStatisticsCalculator(this,
                                            findScalarResultIndex(RiaDefines::STATIC_NATIVE, RiaDefines::riMultYResultName()));
        calc->addNativeStatisticsCalculator(this,
                                            findScalarResultIndex(RiaDefines::STATIC_NATIVE, RiaDefines::riMultZResultName()));
        statisticsCalculator = calc;
    }
    else if (resultName == RiaDefines::combinedRiAreaNormTranResultName())
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();
        calc->addNativeStatisticsCalculator(
            this, findScalarResultIndex(RiaDefines::STATIC_NATIVE, RiaDefines::riAreaNormTranXResultName()));
        calc->addNativeStatisticsCalculator(
            this, findScalarResultIndex(RiaDefines::STATIC_NATIVE, RiaDefines::riAreaNormTranYResultName()));
        calc->addNativeStatisticsCalculator(
            this, findScalarResultIndex(RiaDefines::STATIC_NATIVE, RiaDefines::riAreaNormTranZResultName()));
        statisticsCalculator = calc;
    }
    else if (resultName == RiaDefines::combinedWaterFluxResultName())
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "FLRWATI+"));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "FLRWATJ+"));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "FLRWATK+"));
        statisticsCalculator = calc;
    }
    else if (resultName == RiaDefines::combinedOilFluxResultName())
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "FLROILI+"));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "FLROILJ+"));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "FLROILK+"));
        statisticsCalculator = calc;
    }
    else if (resultName == RiaDefines::combinedGasFluxResultName())
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "FLRGASI+"));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "FLRGASJ+"));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "FLRGASK+"));
        statisticsCalculator = calc;
    }
    else if (resultName.endsWith("IJK"))
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc     = new RigEclipseMultiPropertyStatCalc();
        QString                                   baseName = resultName.left(resultName.size() - 3);
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RiaDefines::GENERATED, QString("%1I").arg(baseName)));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RiaDefines::GENERATED, QString("%1J").arg(baseName)));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RiaDefines::GENERATED, QString("%1K").arg(baseName)));
        statisticsCalculator = calc;
    }
    else
    {
        statisticsCalculator = new RigEclipseNativeStatCalc(this, scalarResultIndex);
    }

    cvf::ref<RigStatisticsDataCache> dataCache = new RigStatisticsDataCache(statisticsCalculator.p());
    m_statisticsDataCache.push_back(dataCache.p());

    return scalarResultIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RigCaseCellResultsData::resultNames(RiaDefines::ResultCatType resType) const
{
    QStringList                                       varList;
    std::vector<RigEclipseResultInfo>::const_iterator it;
    for (it = m_resultInfos.begin(); it != m_resultInfos.end(); ++it)
    {
        if (it->resultType() == resType)
        {
            varList.push_back(it->resultName());
        }
    }
    return varList;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigActiveCellInfo* RigCaseCellResultsData::activeCellInfo()
{
    return m_activeCellInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigActiveCellInfo* RigCaseCellResultsData::activeCellInfo() const
{
    return m_activeCellInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::recalculateStatistics(size_t scalarResultIndex)
{
    m_statisticsDataCache[scalarResultIndex]->clearAllStatistics();
}

//--------------------------------------------------------------------------------------------------
/// Returns whether the result data in question is addressed by Active Cell Index
//--------------------------------------------------------------------------------------------------
bool RigCaseCellResultsData::isUsingGlobalActiveIndex(size_t scalarResultIndex) const
{
    CVF_TIGHT_ASSERT(scalarResultIndex < m_cellScalarResults.size());

    if (!m_cellScalarResults[scalarResultIndex].size()) return true;

    size_t firstTimeStepResultValueCount = m_cellScalarResults[scalarResultIndex][0].size();
    if (firstTimeStepResultValueCount == m_ownerMainGrid->globalCellArray().size()) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCaseCellResultsData::hasFlowDiagUsableFluxes() const
{
    QStringList dynResVarNames = resultNames(RiaDefines::DYNAMIC_NATIVE);

    bool hasFlowFluxes = true;
    hasFlowFluxes      = dynResVarNames.contains("FLRWATI+");
    hasFlowFluxes      = hasFlowFluxes && (dynResVarNames.contains("FLROILI+") || dynResVarNames.contains("FLRGASI+"));

    return hasFlowFluxes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RigCaseCellResultsData::allTimeStepDatesFromEclipseReader() const
{
    const RifReaderEclipseOutput* rifReaderOutput = dynamic_cast<const RifReaderEclipseOutput*>(m_readerInterface.p());
    if (rifReaderOutput)
    {
        return rifReaderOutput->allTimeSteps();
    }
    else
    {
        return std::vector<QDateTime>();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RigCaseCellResultsData::timeStepDates(size_t scalarResultIndex) const
{
    if (scalarResultIndex < m_resultInfos.size())
    {
        return m_resultInfos[scalarResultIndex].dates();
    }
    else
        return std::vector<QDateTime>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RigCaseCellResultsData::timeStepDates() const
{
    size_t scalarResWithMostTimeSteps = cvf::UNDEFINED_SIZE_T;
    maxTimeStepCount(&scalarResWithMostTimeSteps);

    return timeStepDates(scalarResWithMostTimeSteps);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigCaseCellResultsData::daysSinceSimulationStart() const
{
    size_t scalarResWithMostTimeSteps = cvf::UNDEFINED_SIZE_T;
    maxTimeStepCount(&scalarResWithMostTimeSteps);

    return daysSinceSimulationStart(scalarResWithMostTimeSteps);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigCaseCellResultsData::daysSinceSimulationStart(size_t scalarResultIndex) const
{
    if (scalarResultIndex < m_resultInfos.size())
    {
        return m_resultInfos[scalarResultIndex].daysSinceSimulationStarts();
    }
    else
    {
        return std::vector<double>();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigCaseCellResultsData::reportStepNumber(size_t scalarResultIndex, size_t timeStepIndex) const
{
    if (scalarResultIndex < m_resultInfos.size() && m_resultInfos[scalarResultIndex].timeStepInfos().size() > timeStepIndex)
        return m_resultInfos[scalarResultIndex].timeStepInfos()[timeStepIndex].m_reportNumber;
    else
        return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigEclipseTimeStepInfo> RigCaseCellResultsData::timeStepInfos(size_t scalarResultIndex) const
{
    if (scalarResultIndex < m_resultInfos.size())
        return m_resultInfos[scalarResultIndex].timeStepInfos();
    else
        return std::vector<RigEclipseTimeStepInfo>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::setTimeStepInfos(size_t scalarResultIndex, const std::vector<RigEclipseTimeStepInfo>& timeStepInfos)
{
    CVF_ASSERT(scalarResultIndex < m_resultInfos.size());

    m_resultInfos[scalarResultIndex].setTimeStepInfos(timeStepInfos);

    std::vector<std::vector<double>>& dataValues = this->cellScalarResults(scalarResultIndex);
    dataValues.resize(timeStepInfos.size());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::maxTimeStepCount(size_t* scalarResultIndexWithMostTimeSteps) const
{
    size_t maxTsCount                      = 0;
    size_t scalarResultIndexWithMaxTsCount = cvf::UNDEFINED_SIZE_T;

    for (size_t i = 0; i < m_resultInfos.size(); i++)
    {
        if (m_resultInfos[i].timeStepInfos().size() > maxTsCount)
        {
            maxTsCount                      = m_resultInfos[i].timeStepInfos().size();
            scalarResultIndexWithMaxTsCount = i;
        }
    }

    if (scalarResultIndexWithMostTimeSteps)
    {
        *scalarResultIndexWithMostTimeSteps = scalarResultIndexWithMaxTsCount;
    }

    return maxTsCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigCaseCellResultsData::makeResultNameUnique(const QString& resultNameProposal) const
{
    QString newResultName = resultNameProposal;
    size_t  resultIndex   = cvf::UNDEFINED_SIZE_T;
    int     nameNum       = 1;
    int     stringLength  = newResultName.size();
    while (true)
    {
        resultIndex = this->findScalarResultIndex(newResultName);
        if (resultIndex == cvf::UNDEFINED_SIZE_T) break;

        newResultName.truncate(stringLength);
        newResultName += "_" + QString::number(nameNum);
        ++nameNum;
    }

    return newResultName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::clearScalarResult(RiaDefines::ResultCatType type, const QString& resultName)
{
    size_t scalarResultIndex = this->findScalarResultIndex(type, resultName);
    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T) return;

    for (size_t tsIdx = 0; tsIdx < m_cellScalarResults[scalarResultIndex].size(); ++tsIdx)
    {
        std::vector<double> empty;
        m_cellScalarResults[scalarResultIndex][tsIdx].swap(empty);
    }

    recalculateStatistics(scalarResultIndex);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::clearScalarResult(const RigEclipseResultInfo& resultInfo)
{
    clearScalarResult(resultInfo.resultType(), resultInfo.resultName());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::clearAllResults()
{
    m_cellScalarResults.clear();
    m_resultInfos.clear();
    m_statisticsDataCache.clear();
}

//--------------------------------------------------------------------------------------------------
/// Removes all the actual numbers put into this object, and frees up the memory.
/// Does not touch the metadata in any way
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::freeAllocatedResultsData()
{
    for (size_t resultIdx = 0; resultIdx < m_cellScalarResults.size(); ++resultIdx)
    {
        for (size_t tsIdx = 0; tsIdx < m_cellScalarResults[resultIdx].size(); ++tsIdx)
        {
            // Using swap with an empty vector as that is the safest way to really get rid of the allocated data in a vector
            std::vector<double> empty;
            m_cellScalarResults[resultIdx][tsIdx].swap(empty);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCaseCellResultsData::isResultLoaded(const RigEclipseResultInfo& resultInfo) const
{
    size_t scalarResultIndex = this->findScalarResultIndex(resultInfo.resultType(), resultInfo.resultName());
    CVF_TIGHT_ASSERT(scalarResultIndex != cvf::UNDEFINED_SIZE_T);
    if (scalarResultIndex != cvf::UNDEFINED_SIZE_T)
    {
        return isDataPresent(scalarResultIndex);
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// Make sure we have a result with given type and name, and make sure one "timestep" result vector
// for the static result values are allocated
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::addStaticScalarResult(RiaDefines::ResultCatType type,
                                                     const QString&            resultName,
                                                     bool                      needsToBeStored,
                                                     size_t                    resultValueCount)
{
    size_t resultIdx = findOrCreateScalarResultIndex(type, resultName, needsToBeStored);

    m_cellScalarResults[resultIdx].resize(1, std::vector<double>());
    m_cellScalarResults[resultIdx][0].resize(resultValueCount, HUGE_VAL);

    return resultIdx;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCaseCellResultsData::updateResultName(RiaDefines::ResultCatType resultType, QString& oldName, const QString& newName)
{
    bool anyNameUpdated = false;

    for (auto& it : m_resultInfos)
    {
        if (it.resultType() == resultType && it.resultName() == oldName)
        {
            anyNameUpdated = true;
            it.setResultName(newName);
        }
    }

    return anyNameUpdated;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>*
    RigCaseCellResultsData::getResultIndexableStaticResult(RigActiveCellInfo*      actCellInfo,
                                                           RigCaseCellResultsData* gridCellResults,
                                                           QString                 porvResultName,
                                                           std::vector<double>&    activeCellsResultsTempContainer)
{
    size_t resultCellCount    = actCellInfo->reservoirCellResultCount();
    size_t reservoirCellCount = actCellInfo->reservoirCellCount();

    size_t scalarResultIndexPorv = gridCellResults->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, porvResultName);

    if (scalarResultIndexPorv == cvf::UNDEFINED_SIZE_T) return nullptr;

    const std::vector<double>* porvResults = &(gridCellResults->cellScalarResults(scalarResultIndexPorv, 0));

    if (!gridCellResults->isUsingGlobalActiveIndex(scalarResultIndexPorv))
    {
        // PORV is given for all cells

        activeCellsResultsTempContainer.resize(resultCellCount, HUGE_VAL);

        for (size_t globalCellIndex = 0; globalCellIndex < reservoirCellCount; globalCellIndex++)
        {
            size_t resultIdx = actCellInfo->cellResultIndex(globalCellIndex);
            if (resultIdx != cvf::UNDEFINED_SIZE_T)
            {
                activeCellsResultsTempContainer[resultIdx] = porvResults->at(globalCellIndex);
            }
        }
        return &activeCellsResultsTempContainer;
    }
    else
    {
        return porvResults;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RigEclipseResultInfo>& RigCaseCellResultsData::infoForEachResultIndex()
{
    return m_resultInfos;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCaseCellResultsData::mustBeCalculated(size_t scalarResultIndex) const
{
    std::vector<RigEclipseResultInfo>::const_iterator it;
    for (it = m_resultInfos.begin(); it != m_resultInfos.end(); ++it)
    {
        if (it->gridScalarResultIndex() == scalarResultIndex)
        {
            return it->mustBeCalculated();
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::setMustBeCalculated(size_t scalarResultIndex)
{
    std::vector<RigEclipseResultInfo>::iterator it;
    for (it = m_resultInfos.begin(); it != m_resultInfos.end(); ++it)
    {
        if (it->gridScalarResultIndex() == scalarResultIndex)
        {
            it->setMustBeCalculated(true);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::eraseAllSourSimData()
{
    for (size_t i = 0; i < m_resultInfos.size(); i++)
    {
        RigEclipseResultInfo& ri = m_resultInfos[i];
        if (ri.resultType() == RiaDefines::SOURSIMRL)
        {
            ri.setResultType(RiaDefines::REMOVED);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::createPlaceholderResultEntries()
{
    // SOIL
    {
        size_t soilIndex = findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "SOIL");
        if (soilIndex == cvf::UNDEFINED_SIZE_T)
        {
            size_t swatIndex = findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "SWAT");
            size_t sgasIndex = findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "SGAS");

            if (swatIndex != cvf::UNDEFINED_SIZE_T || sgasIndex != cvf::UNDEFINED_SIZE_T)
            {
                soilIndex = findOrCreateScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "SOIL", false);
                this->setMustBeCalculated(soilIndex);
            }
        }
    }

    // Oil Volume
    if (RiaApplication::enableDevelopmentFeatures())
    {
        size_t soilIndex = findOrCreateScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "SOIL", false);
        if (soilIndex != cvf::UNDEFINED_SIZE_T)
        {
            findOrCreateScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, RiaDefines::riOilVolumeResultName(), false);
        }
    }

    // Completion type
    {
        size_t completionTypeIndex = findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, RiaDefines::completionTypeResultName());
        if (completionTypeIndex == cvf::UNDEFINED_SIZE_T)
        {
            findOrCreateScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, RiaDefines::completionTypeResultName(), false);
        }
    }

    // FLUX
    {
        size_t waterIndex = findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, RiaDefines::combinedWaterFluxResultName());
        if (waterIndex == cvf::UNDEFINED_SIZE_T &&
            findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "FLRWATI+") != cvf::UNDEFINED_SIZE_T &&
            findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "FLRWATJ+") != cvf::UNDEFINED_SIZE_T &&
            findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "FLRWATK+") != cvf::UNDEFINED_SIZE_T)
        {
            findOrCreateScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, RiaDefines::combinedWaterFluxResultName(), false);
        }
        size_t oilIndex = findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, RiaDefines::combinedOilFluxResultName());
        if (oilIndex == cvf::UNDEFINED_SIZE_T &&
            findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "FLROILI+") != cvf::UNDEFINED_SIZE_T &&
            findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "FLROILJ+") != cvf::UNDEFINED_SIZE_T &&
            findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "FLROILK+") != cvf::UNDEFINED_SIZE_T)
        {
            findOrCreateScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, RiaDefines::combinedOilFluxResultName(), false);
        }
        size_t gasIndex = findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, RiaDefines::combinedGasFluxResultName());
        if (gasIndex == cvf::UNDEFINED_SIZE_T &&
            findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "FLRGASI+") != cvf::UNDEFINED_SIZE_T &&
            findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "FLRGASJ+") != cvf::UNDEFINED_SIZE_T &&
            findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "FLRGASK+") != cvf::UNDEFINED_SIZE_T)
        {
            findOrCreateScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, RiaDefines::combinedGasFluxResultName(), false);
        }
    }

    // TRANSXYZ
    {
        size_t tranX, tranY, tranZ;
        if (findTransmissibilityResults(tranX, tranY, tranZ))
        {
            addStaticScalarResult(RiaDefines::STATIC_NATIVE, RiaDefines::combinedTransmissibilityResultName(), false, 0);
        }
    }
    // MULTXYZ
    {
        addStaticScalarResult(RiaDefines::STATIC_NATIVE, RiaDefines::combinedMultResultName(), false, 0);
    }

    // riTRANSXYZ and X,Y,Z
    {
        if (findScalarResultIndex(RiaDefines::STATIC_NATIVE, "PERMX") != cvf::UNDEFINED_SIZE_T &&
            findScalarResultIndex(RiaDefines::STATIC_NATIVE, "PERMY") != cvf::UNDEFINED_SIZE_T &&
            findScalarResultIndex(RiaDefines::STATIC_NATIVE, "PERMZ") != cvf::UNDEFINED_SIZE_T)
        {
            addStaticScalarResult(RiaDefines::STATIC_NATIVE, RiaDefines::riTranXResultName(), false, 0);
            addStaticScalarResult(RiaDefines::STATIC_NATIVE, RiaDefines::riTranYResultName(), false, 0);
            addStaticScalarResult(RiaDefines::STATIC_NATIVE, RiaDefines::riTranZResultName(), false, 0);
            addStaticScalarResult(RiaDefines::STATIC_NATIVE, RiaDefines::combinedRiTranResultName(), false, 0);
        }
    }

    // riMULTXYZ and X, Y, Z
    {
        size_t tranX, tranY, tranZ;
        if (findTransmissibilityResults(tranX, tranY, tranZ) &&
            findScalarResultIndex(RiaDefines::STATIC_NATIVE, RiaDefines::riTranXResultName()) != cvf::UNDEFINED_SIZE_T &&
            findScalarResultIndex(RiaDefines::STATIC_NATIVE, RiaDefines::riTranYResultName()) != cvf::UNDEFINED_SIZE_T &&
            findScalarResultIndex(RiaDefines::STATIC_NATIVE, RiaDefines::riTranZResultName()) != cvf::UNDEFINED_SIZE_T)
        {
            addStaticScalarResult(RiaDefines::STATIC_NATIVE, RiaDefines::riMultXResultName(), false, 0);
            addStaticScalarResult(RiaDefines::STATIC_NATIVE, RiaDefines::riMultYResultName(), false, 0);
            addStaticScalarResult(RiaDefines::STATIC_NATIVE, RiaDefines::riMultZResultName(), false, 0);
            addStaticScalarResult(RiaDefines::STATIC_NATIVE, RiaDefines::combinedRiMultResultName(), false, 0);
        }
    }

    // riTRANSXYZbyArea and X, Y, Z
    {
        if (findScalarResultIndex(RiaDefines::STATIC_NATIVE, "TRANX") != cvf::UNDEFINED_SIZE_T)
        {
            addStaticScalarResult(RiaDefines::STATIC_NATIVE, RiaDefines::riAreaNormTranXResultName(), false, 0);
        }

        if (findScalarResultIndex(RiaDefines::STATIC_NATIVE, "TRANY") != cvf::UNDEFINED_SIZE_T)
        {
            addStaticScalarResult(RiaDefines::STATIC_NATIVE, RiaDefines::riAreaNormTranYResultName(), false, 0);
        }

        if (findScalarResultIndex(RiaDefines::STATIC_NATIVE, "TRANZ") != cvf::UNDEFINED_SIZE_T)
        {
            addStaticScalarResult(RiaDefines::STATIC_NATIVE, RiaDefines::riAreaNormTranZResultName(), false, 0);
        }

        size_t tranX, tranY, tranZ;
        if (findTransmissibilityResults(tranX, tranY, tranZ))
        {
            addStaticScalarResult(RiaDefines::STATIC_NATIVE, RiaDefines::combinedRiAreaNormTranResultName(), false, 0);
        }
    }

    // Cell Volume
    {
        addStaticScalarResult(RiaDefines::STATIC_NATIVE, RiaDefines::riCellVolumeResultName(), false, 0);
    }

    // Mobile Pore Volume
    {
        if (findScalarResultIndex(RiaDefines::STATIC_NATIVE, "PORV") != cvf::UNDEFINED_SIZE_T)
        {
            addStaticScalarResult(RiaDefines::STATIC_NATIVE, RiaDefines::mobilePoreVolumeName(), false, 0);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCaseCellResultsData::findTransmissibilityResults(size_t& tranX, size_t& tranY, size_t& tranZ) const
{
    tranX = findScalarResultIndex(RiaDefines::STATIC_NATIVE, "TRANX");
    tranY = findScalarResultIndex(RiaDefines::STATIC_NATIVE, "TRANY");
    tranZ = findScalarResultIndex(RiaDefines::STATIC_NATIVE, "TRANZ");

    if (tranX == cvf::UNDEFINED_SIZE_T || tranY == cvf::UNDEFINED_SIZE_T || tranZ == cvf::UNDEFINED_SIZE_T)
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::findOrLoadScalarResult(const QString& resultName)
{
    size_t scalarResultIndex = this->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, resultName);

    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        scalarResultIndex = this->findOrLoadScalarResult(RiaDefines::DYNAMIC_NATIVE, resultName);
    }

    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        scalarResultIndex = this->findOrLoadScalarResult(RiaDefines::SOURSIMRL, resultName);
    }

    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        scalarResultIndex = this->findScalarResultIndex(RiaDefines::GENERATED, resultName);
    }

    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        scalarResultIndex = this->findScalarResultIndex(RiaDefines::INPUT_PROPERTY, resultName);
    }

    return scalarResultIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::findOrLoadScalarResult(RiaDefines::ResultCatType type, const QString& resultName)
{
    size_t scalarResultIndex = this->findScalarResultIndex(type, resultName);
    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T) return cvf::UNDEFINED_SIZE_T;

    // Load dependency data sets

    if (type == RiaDefines::STATIC_NATIVE)
    {
        if (resultName == RiaDefines::combinedTransmissibilityResultName())
        {
            this->findOrLoadScalarResult(type, "TRANX");
            this->findOrLoadScalarResult(type, "TRANY");
            this->findOrLoadScalarResult(type, "TRANZ");
        }
        else if (resultName == RiaDefines::combinedMultResultName())
        {
            this->findOrLoadScalarResult(type, "MULTX");
            this->findOrLoadScalarResult(type, "MULTX-");
            this->findOrLoadScalarResult(type, "MULTY");
            this->findOrLoadScalarResult(type, "MULTY-");
            this->findOrLoadScalarResult(type, "MULTZ");
            this->findOrLoadScalarResult(type, "MULTZ-");
        }
        else if (resultName == RiaDefines::combinedRiTranResultName())
        {
            computeRiTransComponent(RiaDefines::riTranXResultName());
            computeRiTransComponent(RiaDefines::riTranYResultName());
            computeRiTransComponent(RiaDefines::riTranZResultName());
            computeNncCombRiTrans();
        }
        else if (resultName == RiaDefines::riTranXResultName() || resultName == RiaDefines::riTranYResultName() ||
                 resultName == RiaDefines::riTranZResultName())
        {
            computeRiTransComponent(resultName);
        }
        else if (resultName == RiaDefines::combinedRiMultResultName())
        {
            computeRiMULTComponent(RiaDefines::riMultXResultName());
            computeRiMULTComponent(RiaDefines::riMultYResultName());
            computeRiMULTComponent(RiaDefines::riMultZResultName());
            computeNncCombRiTrans();
            computeNncCombRiMULT();
        }
        else if (resultName == RiaDefines::riMultXResultName() || resultName == RiaDefines::riMultYResultName() ||
                 resultName == RiaDefines::riMultZResultName())
        {
            computeRiMULTComponent(resultName);
        }
        else if (resultName == RiaDefines::combinedRiAreaNormTranResultName())
        {
            computeRiTRANSbyAreaComponent(RiaDefines::riAreaNormTranXResultName());
            computeRiTRANSbyAreaComponent(RiaDefines::riAreaNormTranYResultName());
            computeRiTRANSbyAreaComponent(RiaDefines::riAreaNormTranZResultName());
            computeNncCombRiTRANSbyArea();
        }
        else if (resultName == RiaDefines::riAreaNormTranXResultName() || resultName == RiaDefines::riAreaNormTranYResultName() ||
                 resultName == RiaDefines::riAreaNormTranZResultName())
        {
            computeRiTRANSbyAreaComponent(resultName);
        }
    }
    else if (type == RiaDefines::DYNAMIC_NATIVE)
    {
        if (resultName == RiaDefines::combinedWaterFluxResultName())
        {
            this->findOrLoadScalarResult(type, "FLRWATI+");
            this->findOrLoadScalarResult(type, "FLRWATJ+");
            this->findOrLoadScalarResult(type, "FLRWATK+");
        }
        else if (resultName == RiaDefines::combinedOilFluxResultName())
        {
            this->findOrLoadScalarResult(type, "FLROILI+");
            this->findOrLoadScalarResult(type, "FLROILJ+");
            this->findOrLoadScalarResult(type, "FLROILK+");
        }
        else if (resultName == RiaDefines::combinedGasFluxResultName())
        {
            this->findOrLoadScalarResult(type, "FLRGASI+");
            this->findOrLoadScalarResult(type, "FLRGASJ+");
            this->findOrLoadScalarResult(type, "FLRGASK+");
        }
    }

    if (isDataPresent(scalarResultIndex))
    {
        return scalarResultIndex;
    }

    if (resultName == "SOIL")
    {
        if (this->mustBeCalculated(scalarResultIndex))
        {
            // Trigger loading of SWAT, SGAS to establish time step count if no data has been loaded from file at this point
            findOrLoadScalarResult(RiaDefines::DYNAMIC_NATIVE, "SWAT");
            findOrLoadScalarResult(RiaDefines::DYNAMIC_NATIVE, "SGAS");

            this->cellScalarResults(scalarResultIndex).resize(this->maxTimeStepCount());
            for (size_t timeStepIdx = 0; timeStepIdx < this->maxTimeStepCount(); timeStepIdx++)
            {
                std::vector<double>& values = this->cellScalarResults(scalarResultIndex)[timeStepIdx];
                if (values.size() == 0)
                {
                    computeSOILForTimeStep(timeStepIdx);
                }
            }

            return scalarResultIndex;
        }
    }
    else if (resultName == RiaDefines::completionTypeResultName())
    {
        caf::ProgressInfo progressInfo(this->maxTimeStepCount(), "Calculate Completion Type Results");
        this->cellScalarResults(scalarResultIndex).resize(this->maxTimeStepCount());
        for (size_t timeStepIdx = 0; timeStepIdx < this->maxTimeStepCount(); ++timeStepIdx)
        {
            computeCompletionTypeForTimeStep(timeStepIdx);
            progressInfo.incrementProgress();
        }
    }
    else if (resultName == RiaDefines::mobilePoreVolumeName())
    {
        computeMobilePV();
    }

    if (type == RiaDefines::GENERATED)
    {
        return cvf::UNDEFINED_SIZE_T;
    }

    if (m_readerInterface.notNull())
    {
        // Add one more result to result container
        size_t timeStepCount = this->infoForEachResultIndex()[scalarResultIndex].timeStepInfos().size();

        bool resultLoadingSucess = true;

        size_t tempGridCellCount = m_ownerMainGrid->totalTemporaryGridCellCount();

        if (type == RiaDefines::DYNAMIC_NATIVE && timeStepCount > 0)
        {
            this->cellScalarResults(scalarResultIndex).resize(timeStepCount);

            size_t i;
            for (i = 0; i < timeStepCount; i++)
            {
                std::vector<double>& values = this->cellScalarResults(scalarResultIndex)[i];
                if (!m_readerInterface->dynamicResult(resultName, RiaDefines::MATRIX_MODEL, i, &values))
                {
                    resultLoadingSucess = false;
                }
                else if (tempGridCellCount > 0)
                {
                    if (!values.empty())
                    {
                        values.resize(values.size() + tempGridCellCount, std::numeric_limits<double>::infinity());

                        assignValuesToTemporaryLgrs(resultName, values);
                    }
                }
            }
        }
        else if (type == RiaDefines::STATIC_NATIVE)
        {
            this->cellScalarResults(scalarResultIndex).resize(1);

            std::vector<double>& values = this->cellScalarResults(scalarResultIndex)[0];
            if (!m_readerInterface->staticResult(resultName, RiaDefines::MATRIX_MODEL, &values))
            {
                resultLoadingSucess = false;
            }
            else if (tempGridCellCount > 0)
            {
                if (!values.empty())
                {
                    values.resize(values.size() + tempGridCellCount, std::numeric_limits<double>::infinity());

                    assignValuesToTemporaryLgrs(resultName, values);
                }
            }
        }

        if (!resultLoadingSucess)
        {
            // Remove last scalar result because loading of result failed
            this->cellScalarResults(scalarResultIndex).clear();
        }
    }


    if (resultName == RiaDefines::riCellVolumeResultName())
    {
        computeCellVolumes();
    }
    else if (resultName == RiaDefines::riOilVolumeResultName())
    {
        computeCellVolumes();
        computeOilVolumes();
    }


    // Handle SourSimRL reading

    if (type == RiaDefines::SOURSIMRL)
    {
        RifReaderEclipseOutput* eclReader = dynamic_cast<RifReaderEclipseOutput*>(m_readerInterface.p());
        if (eclReader)
        {
            size_t timeStepCount = this->infoForEachResultIndex()[scalarResultIndex].timeStepInfos().size();

            this->cellScalarResults(scalarResultIndex).resize(timeStepCount);

            size_t i;
            for (i = 0; i < timeStepCount; i++)
            {
                std::vector<double>& values = this->cellScalarResults(scalarResultIndex)[i];
                eclReader->sourSimRlResult(resultName, i, &values);
            }
        }
    }

    return scalarResultIndex;
}

//--------------------------------------------------------------------------------------------------
/// This method is intended to be used for multicase cross statistical calculations, when
/// we need process one timestep at a time, freeing memory as we go.
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::findOrLoadScalarResultForTimeStep(RiaDefines::ResultCatType type,
                                                                 const QString&            resultName,
                                                                 size_t                    timeStepIndex)
{
    // Special handling for SOIL
    if (type == RiaDefines::DYNAMIC_NATIVE && resultName.toUpper() == "SOIL")
    {
        size_t soilScalarResultIndex = this->findScalarResultIndex(type, resultName);

        if (this->mustBeCalculated(soilScalarResultIndex))
        {
            this->cellScalarResults(soilScalarResultIndex).resize(this->maxTimeStepCount());

            std::vector<double>& values = this->cellScalarResults(soilScalarResultIndex)[timeStepIndex];
            if (values.size() == 0)
            {
                computeSOILForTimeStep(timeStepIndex);
            }

            return soilScalarResultIndex;
        }
    }
    else if (type == RiaDefines::DYNAMIC_NATIVE && resultName == RiaDefines::completionTypeResultName())
    {
        size_t completionTypeScalarResultIndex = this->findScalarResultIndex(type, resultName);
        computeCompletionTypeForTimeStep(timeStepIndex);
        return completionTypeScalarResultIndex;
    }

    size_t scalarResultIndex = this->findScalarResultIndex(type, resultName);
    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T) return cvf::UNDEFINED_SIZE_T;

    if (type == RiaDefines::GENERATED)
    {
        return cvf::UNDEFINED_SIZE_T;
    }

    if (m_readerInterface.notNull())
    {
        size_t timeStepCount = this->infoForEachResultIndex()[scalarResultIndex].timeStepInfos().size();

        bool resultLoadingSucess = true;

        if (type == RiaDefines::DYNAMIC_NATIVE && timeStepCount > 0)
        {
            this->cellScalarResults(scalarResultIndex).resize(timeStepCount);

            std::vector<double>& values = this->cellScalarResults(scalarResultIndex)[timeStepIndex];
            if (values.size() == 0)
            {
                if (!m_readerInterface->dynamicResult(resultName, RiaDefines::MATRIX_MODEL, timeStepIndex, &values))
                {
                    resultLoadingSucess = false;
                }
            }
        }
        else if (type == RiaDefines::STATIC_NATIVE)
        {
            this->cellScalarResults(scalarResultIndex).resize(1);

            std::vector<double>& values = this->cellScalarResults(scalarResultIndex)[0];
            if (!m_readerInterface->staticResult(resultName, RiaDefines::MATRIX_MODEL, &values))
            {
                resultLoadingSucess = false;
            }
        }

        if (!resultLoadingSucess)
        {
            // Error logging
            CVF_ASSERT(false);
        }
    }

    // Handle SourSimRL reading

    if (type == RiaDefines::SOURSIMRL)
    {
        RifReaderEclipseOutput* eclReader = dynamic_cast<RifReaderEclipseOutput*>(m_readerInterface.p());
        if (eclReader)
        {
            size_t timeStepCount = this->infoForEachResultIndex()[scalarResultIndex].timeStepInfos().size();

            this->cellScalarResults(scalarResultIndex).resize(timeStepCount);

            std::vector<double>& values = this->cellScalarResults(scalarResultIndex)[timeStepIndex];

            if (values.size() == 0)
            {
                eclReader->sourSimRlResult(resultName, timeStepIndex, &values);
            }
        }
    }

    return scalarResultIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeSOILForTimeStep(size_t timeStepIndex)
{
    // Compute SGAS based on SWAT if the simulation contains no oil
    testAndComputeSgasForTimeStep(timeStepIndex);

    size_t scalarIndexSWAT = findOrLoadScalarResultForTimeStep(RiaDefines::DYNAMIC_NATIVE, "SWAT", timeStepIndex);
    size_t scalarIndexSGAS = findOrLoadScalarResultForTimeStep(RiaDefines::DYNAMIC_NATIVE, "SGAS", timeStepIndex);
    size_t scalarIndexSSOL = findOrLoadScalarResultForTimeStep(RiaDefines::DYNAMIC_NATIVE, "SSOL", timeStepIndex);

    // Early exit if none of SWAT or SGAS is present
    if (scalarIndexSWAT == cvf::UNDEFINED_SIZE_T && scalarIndexSGAS == cvf::UNDEFINED_SIZE_T)
    {
        return;
    }

    size_t soilResultValueCount = 0;
    size_t soilTimeStepCount    = 0;

    if (scalarIndexSWAT != cvf::UNDEFINED_SIZE_T)
    {
        std::vector<double>& swatForTimeStep = this->cellScalarResults(scalarIndexSWAT, timeStepIndex);
        if (swatForTimeStep.size() > 0)
        {
            soilResultValueCount = swatForTimeStep.size();
            soilTimeStepCount    = this->infoForEachResultIndex()[scalarIndexSWAT].timeStepInfos().size();
        }
    }

    if (scalarIndexSGAS != cvf::UNDEFINED_SIZE_T)
    {
        std::vector<double>& sgasForTimeStep = this->cellScalarResults(scalarIndexSGAS, timeStepIndex);
        if (sgasForTimeStep.size() > 0)
        {
            soilResultValueCount = qMax(soilResultValueCount, sgasForTimeStep.size());

            size_t sgasTimeStepCount = this->infoForEachResultIndex()[scalarIndexSGAS].timeStepInfos().size();
            soilTimeStepCount        = qMax(soilTimeStepCount, sgasTimeStepCount);
        }
    }

    // Make sure memory is allocated for the new SOIL results

    size_t soilResultScalarIndex = this->findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "SOIL");
    this->cellScalarResults(soilResultScalarIndex).resize(soilTimeStepCount);

    if (this->cellScalarResults(soilResultScalarIndex, timeStepIndex).size() > 0)
    {
        // Data is computed and allocated, nothing more to do
        return;
    }

    this->cellScalarResults(soilResultScalarIndex, timeStepIndex).resize(soilResultValueCount);

    std::vector<double>* swatForTimeStep = nullptr;
    std::vector<double>* sgasForTimeStep = nullptr;
    std::vector<double>* ssolForTimeStep = nullptr;

    if (scalarIndexSWAT != cvf::UNDEFINED_SIZE_T)
    {
        swatForTimeStep = &(this->cellScalarResults(scalarIndexSWAT, timeStepIndex));
        if (swatForTimeStep->size() == 0)
        {
            swatForTimeStep = nullptr;
        }
    }

    if (scalarIndexSGAS != cvf::UNDEFINED_SIZE_T)
    {
        sgasForTimeStep = &(this->cellScalarResults(scalarIndexSGAS, timeStepIndex));
        if (sgasForTimeStep->size() == 0)
        {
            sgasForTimeStep = nullptr;
        }
    }

    if (scalarIndexSSOL != cvf::UNDEFINED_SIZE_T)
    {
        ssolForTimeStep = &(this->cellScalarResults(scalarIndexSSOL, timeStepIndex));
        if (ssolForTimeStep->size() == 0)
        {
            ssolForTimeStep = nullptr;
        }
    }

    std::vector<double>& soilForTimeStep = this->cellScalarResults(soilResultScalarIndex, timeStepIndex);

#pragma omp parallel for
    for (int idx = 0; idx < static_cast<int>(soilResultValueCount); idx++)
    {
        double soilValue = 1.0;
        if (sgasForTimeStep)
        {
            soilValue -= sgasForTimeStep->at(idx);
        }

        if (swatForTimeStep)
        {
            soilValue -= swatForTimeStep->at(idx);
        }

        if (ssolForTimeStep)
        {
            soilValue -= ssolForTimeStep->at(idx);
        }

        soilForTimeStep[idx] = soilValue;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::testAndComputeSgasForTimeStep(size_t timeStepIndex)
{
    size_t scalarIndexSWAT = findOrLoadScalarResultForTimeStep(RiaDefines::DYNAMIC_NATIVE, "SWAT", timeStepIndex);
    if (scalarIndexSWAT == cvf::UNDEFINED_SIZE_T)
    {
        return;
    }

    if (m_readerInterface.isNull()) return;

    std::set<RiaDefines::PhaseType> phases = m_readerInterface->availablePhases();
    if (phases.count(RiaDefines::GAS_PHASE) == 0) return;
    if (phases.count(RiaDefines::OIL_PHASE) > 0) return;

    // Simulation type is gas and water. No SGAS is present, compute SGAS based on SWAT

    size_t scalarIndexSGAS = this->findOrCreateScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, "SGAS", false);
    if (this->cellScalarResults(scalarIndexSGAS).size() > timeStepIndex)
    {
        std::vector<double>& values = this->cellScalarResults(scalarIndexSGAS)[timeStepIndex];
        if (values.size() > 0) return;
    }

    size_t swatResultValueCount = 0;
    size_t swatTimeStepCount    = 0;

    {
        std::vector<double>& swatForTimeStep = this->cellScalarResults(scalarIndexSWAT, timeStepIndex);
        if (swatForTimeStep.size() > 0)
        {
            swatResultValueCount = swatForTimeStep.size();
            swatTimeStepCount    = this->infoForEachResultIndex()[scalarIndexSWAT].timeStepInfos().size();
        }
    }

    this->cellScalarResults(scalarIndexSGAS).resize(swatTimeStepCount);

    if (this->cellScalarResults(scalarIndexSGAS, timeStepIndex).size() > 0)
    {
        return;
    }

    this->cellScalarResults(scalarIndexSGAS, timeStepIndex).resize(swatResultValueCount);

    std::vector<double>* swatForTimeStep = nullptr;

    {
        swatForTimeStep = &(this->cellScalarResults(scalarIndexSWAT, timeStepIndex));
        if (swatForTimeStep->size() == 0)
        {
            swatForTimeStep = nullptr;
        }
    }

    std::vector<double>& sgasForTimeStep = this->cellScalarResults(scalarIndexSGAS, timeStepIndex);

#pragma omp parallel for
    for (int idx = 0; idx < static_cast<int>(swatResultValueCount); idx++)
    {
        double sgasValue = 1.0;

        if (swatForTimeStep)
        {
            sgasValue -= swatForTimeStep->at(idx);
        }

        sgasForTimeStep[idx] = sgasValue;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeDepthRelatedResults()
{
    size_t actCellCount = activeCellInfo()->reservoirActiveCellCount();
    if (actCellCount == 0) return;

    size_t depthResultIndex  = findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "DEPTH");
    size_t dxResultIndex     = findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "DX");
    size_t dyResultIndex     = findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "DY");
    size_t dzResultIndex     = findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "DZ");
    size_t topsResultIndex   = findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "TOPS");
    size_t bottomResultIndex = findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "BOTTOM");

    bool computeDepth  = false;
    bool computeDx     = false;
    bool computeDy     = false;
    bool computeDz     = false;
    bool computeTops   = false;
    bool computeBottom = false;

    if (depthResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        depthResultIndex = this->addStaticScalarResult(RiaDefines::STATIC_NATIVE, "DEPTH", false, actCellCount);
        computeDepth     = true;
    }

    if (dxResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        dxResultIndex = this->addStaticScalarResult(RiaDefines::STATIC_NATIVE, "DX", false, actCellCount);
        computeDx     = true;
    }

    if (dyResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        dyResultIndex = this->addStaticScalarResult(RiaDefines::STATIC_NATIVE, "DY", false, actCellCount);
        computeDy     = true;
    }

    if (dzResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        dzResultIndex = this->addStaticScalarResult(RiaDefines::STATIC_NATIVE, "DZ", false, actCellCount);
        computeDz     = true;
    }

    if (topsResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        topsResultIndex = this->addStaticScalarResult(RiaDefines::STATIC_NATIVE, "TOPS", false, actCellCount);
        computeTops     = true;
    }

    if (bottomResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        bottomResultIndex = this->addStaticScalarResult(RiaDefines::STATIC_NATIVE, "BOTTOM", false, actCellCount);
        computeBottom     = true;
    }

    std::vector<std::vector<double>>& depth  = this->cellScalarResults(depthResultIndex);
    std::vector<std::vector<double>>& dx     = this->cellScalarResults(dxResultIndex);
    std::vector<std::vector<double>>& dy     = this->cellScalarResults(dyResultIndex);
    std::vector<std::vector<double>>& dz     = this->cellScalarResults(dzResultIndex);
    std::vector<std::vector<double>>& tops   = this->cellScalarResults(topsResultIndex);
    std::vector<std::vector<double>>& bottom = this->cellScalarResults(bottomResultIndex);

    // Make sure the size is at least active cells
    {
        if (depth[0].size() < actCellCount)
        {
            depth[0].resize(actCellCount, std::numeric_limits<double>::max());
            computeDepth = true;
        }

        if (dx[0].size() < actCellCount)
        {
            dx[0].resize(actCellCount, std::numeric_limits<double>::max());
            computeDx = true;
        }

        if (dy[0].size() < actCellCount)
        {
            dy[0].resize(actCellCount, std::numeric_limits<double>::max());
            computeDy = true;
        }

        if (dz[0].size() < actCellCount)
        {
            dz[0].resize(actCellCount, std::numeric_limits<double>::max());
            computeDz = true;
        }

        if (tops[0].size() < actCellCount)
        {
            tops[0].resize(actCellCount, std::numeric_limits<double>::max());
            computeTops = true;
        }

        if (bottom[0].size() < actCellCount)
        {
            bottom[0].resize(actCellCount, std::numeric_limits<double>::max());
            computeBottom = true;
        }
    }

    for (size_t cellIdx = 0; cellIdx < m_ownerMainGrid->globalCellArray().size(); cellIdx++)
    {
        const RigCell& cell = m_ownerMainGrid->globalCellArray()[cellIdx];

        size_t resultIndex = activeCellInfo()->cellResultIndex(cellIdx);
        if (resultIndex == cvf::UNDEFINED_SIZE_T) continue;

        bool isTemporaryGrid = cell.hostGrid()->isTempGrid();

        if (computeDepth || isTemporaryGrid)
        {
            depth[0][resultIndex] = cvf::Math::abs(cell.center().z());
        }

        if (computeDx || isTemporaryGrid)
        {
            cvf::Vec3d cellWidth =
                cell.faceCenter(cvf::StructGridInterface::NEG_I) - cell.faceCenter(cvf::StructGridInterface::POS_I);
            dx[0][resultIndex] = cellWidth.length();
        }

        if (computeDy || isTemporaryGrid)
        {
            cvf::Vec3d cellWidth =
                cell.faceCenter(cvf::StructGridInterface::NEG_J) - cell.faceCenter(cvf::StructGridInterface::POS_J);
            dy[0][resultIndex] = cellWidth.length();
        }

        if (computeDz || isTemporaryGrid)
        {
            cvf::Vec3d cellWidth =
                cell.faceCenter(cvf::StructGridInterface::NEG_K) - cell.faceCenter(cvf::StructGridInterface::POS_K);
            dz[0][resultIndex] = cellWidth.length();
        }

        if (computeTops || isTemporaryGrid)
        {
            tops[0][resultIndex] = cvf::Math::abs(cell.faceCenter(cvf::StructGridInterface::NEG_K).z());
        }

        if (computeBottom || isTemporaryGrid)
        {
            bottom[0][resultIndex] = cvf::Math::abs(cell.faceCenter(cvf::StructGridInterface::POS_K).z());
        }
    }
}

namespace RigTransmissibilityCalcTools
{
void calculateConnectionGeometry(const RigCell&                     c1,
                                 const RigCell&                     c2,
                                 const std::vector<cvf::Vec3d>&     nodes,
                                 cvf::StructGridInterface::FaceType faceId,
                                 cvf::Vec3d*                        faceAreaVec)
{
    CVF_TIGHT_ASSERT(faceAreaVec);

    *faceAreaVec = cvf::Vec3d::ZERO;

    std::vector<size_t>     polygon;
    std::vector<cvf::Vec3d> intersections;
    std::array<size_t, 4>   face1;
    std::array<size_t, 4>   face2;
    c1.faceIndices(faceId, &face1);
    c2.faceIndices(cvf::StructGridInterface::oppositeFace(faceId), &face2);

    bool foundOverlap = cvf::GeometryTools::calculateOverlapPolygonOfTwoQuads(&polygon,
                                                                              &intersections,
                                                                              (cvf::EdgeIntersectStorage<size_t>*)nullptr,
                                                                              cvf::wrapArrayConst(&nodes),
                                                                              face1.data(),
                                                                              face2.data(),
                                                                              1e-6);

    if (foundOverlap)
    {
        std::vector<cvf::Vec3d> realPolygon;

        for (size_t pIdx = 0; pIdx < polygon.size(); ++pIdx)
        {
            if (polygon[pIdx] < nodes.size())
                realPolygon.push_back(nodes[polygon[pIdx]]);
            else
                realPolygon.push_back(intersections[polygon[pIdx] - nodes.size()]);
        }

        // Polygon area vector

        *faceAreaVec = cvf::GeometryTools::polygonAreaNormal3D(realPolygon);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double halfCellTransmissibility(double perm, double ntg, const cvf::Vec3d& centerToFace, const cvf::Vec3d& faceAreaVec)
{
    return perm * ntg * (faceAreaVec * centerToFace) / (centerToFace * centerToFace);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double newtran(double cdarchy, double mult, double halfCellTrans, double neighborHalfCellTrans)
{
    if (cvf::Math::abs(halfCellTrans) < 1e-15 || cvf::Math::abs(neighborHalfCellTrans) < 1e-15)
    {
        return 0.0;
    }

    double result = cdarchy * mult / ((1 / halfCellTrans) + (1 / neighborHalfCellTrans));
    CVF_TIGHT_ASSERT(result == result);
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
typedef size_t (*ResultIndexFunction)(const RigActiveCellInfo* activeCellinfo, size_t reservoirCellIndex);

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

size_t directReservoirCellIndex(const RigActiveCellInfo* activeCellinfo, size_t reservoirCellIndex)
{
    return reservoirCellIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

size_t reservoirActiveCellIndex(const RigActiveCellInfo* activeCellinfo, size_t reservoirCellIndex)
{
    return activeCellinfo->cellResultIndex(reservoirCellIndex);
}
} // namespace RigTransmissibilityCalcTools

using namespace RigTransmissibilityCalcTools;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeRiTransComponent(const QString& riTransComponentResultName)
{
    // Set up which component to compute

    cvf::StructGridInterface::FaceType faceId = cvf::StructGridInterface::NO_FACE;
    QString                            permCompName;

    if (riTransComponentResultName == RiaDefines::riTranXResultName())
    {
        permCompName = "PERMX";
        faceId       = cvf::StructGridInterface::POS_I;
    }
    else if (riTransComponentResultName == RiaDefines::riTranYResultName())
    {
        permCompName = "PERMY";
        faceId       = cvf::StructGridInterface::POS_J;
    }
    else if (riTransComponentResultName == RiaDefines::riTranZResultName())
    {
        permCompName = "PERMZ";
        faceId       = cvf::StructGridInterface::POS_K;
    }
    else
    {
        CVF_ASSERT(false);
    }

    double cdarchy = darchysValue();

    // Get the needed result indices we depend on

    size_t permResultIdx = findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, permCompName);
    size_t ntgResultIdx  = findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "NTG");

    bool hasNTGResults = ntgResultIdx != cvf::UNDEFINED_SIZE_T;

    // Get the result index of the output

    size_t riTransResultIdx = this->findScalarResultIndex(RiaDefines::STATIC_NATIVE, riTransComponentResultName);
    CVF_ASSERT(riTransResultIdx != cvf::UNDEFINED_SIZE_T);

    // Get the result count, to handle that one of them might be globally defined

    size_t permxResultValueCount = this->cellScalarResults(permResultIdx)[0].size();
    size_t resultValueCount      = permxResultValueCount;
    if (hasNTGResults)
    {
        size_t ntgResultValueCount = this->cellScalarResults(ntgResultIdx)[0].size();
        resultValueCount           = CVF_MIN(permxResultValueCount, ntgResultValueCount);
    }

    // Get all the actual result values

    std::vector<double>& permResults    = this->cellScalarResults(permResultIdx)[0];
    std::vector<double>& riTransResults = this->cellScalarResults(riTransResultIdx)[0];
    std::vector<double>* ntgResults     = nullptr;
    if (hasNTGResults)
    {
        ntgResults = &(this->cellScalarResults(ntgResultIdx)[0]);
    }

    // Set up output container to correct number of results

    riTransResults.resize(resultValueCount);

    // Prepare how to index the result values:
    ResultIndexFunction riTranIdxFunc = nullptr;
    ResultIndexFunction permIdxFunc   = nullptr;
    ResultIndexFunction ntgIdxFunc    = nullptr;
    {
        bool isPermUsingResIdx  = this->isUsingGlobalActiveIndex(permResultIdx);
        bool isTransUsingResIdx = this->isUsingGlobalActiveIndex(riTransResultIdx);
        bool isNtgUsingResIdx   = false;
        if (hasNTGResults)
        {
            isNtgUsingResIdx = this->isUsingGlobalActiveIndex(ntgResultIdx);
        }

        // Set up result index function pointers

        riTranIdxFunc = isTransUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;
        permIdxFunc   = isPermUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;
        if (hasNTGResults)
        {
            ntgIdxFunc = isNtgUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;
        }
    }

    const RigActiveCellInfo*       activeCellInfo        = this->activeCellInfo();
    const std::vector<cvf::Vec3d>& nodes                 = m_ownerMainGrid->nodes();
    bool                           isFaceNormalsOutwards = m_ownerMainGrid->isFaceNormalsOutwards();

    for (size_t nativeResvCellIndex = 0; nativeResvCellIndex < m_ownerMainGrid->globalCellArray().size(); nativeResvCellIndex++)
    {
        // Do nothing if we are only dealing with active cells, and this cell is not active:
        size_t tranResIdx = (*riTranIdxFunc)(activeCellInfo, nativeResvCellIndex);

        if (tranResIdx == cvf::UNDEFINED_SIZE_T) continue;

        const RigCell& nativeCell = m_ownerMainGrid->globalCellArray()[nativeResvCellIndex];
        RigGridBase*   grid       = nativeCell.hostGrid();

        size_t gridLocalNativeCellIndex = nativeCell.gridLocalCellIndex();

        size_t i, j, k, gridLocalNeighborCellIdx;

        grid->ijkFromCellIndex(gridLocalNativeCellIndex, &i, &j, &k);

        if (grid->cellIJKNeighbor(i, j, k, faceId, &gridLocalNeighborCellIdx))
        {
            size_t         neighborResvCellIdx = grid->reservoirCellIndex(gridLocalNeighborCellIdx);
            const RigCell& neighborCell        = m_ownerMainGrid->globalCellArray()[neighborResvCellIdx];

            // Do nothing if neighbor cell has no results
            size_t neighborCellPermResIdx = (*permIdxFunc)(activeCellInfo, neighborResvCellIdx);
            if (neighborCellPermResIdx == cvf::UNDEFINED_SIZE_T) continue;

            // Connection geometry

            const RigFault* fault     = grid->mainGrid()->findFaultFromCellIndexAndCellFace(nativeResvCellIndex, faceId);
            bool            isOnFault = fault;

            cvf::Vec3d faceAreaVec;
            cvf::Vec3d faceCenter;

            if (isOnFault)
            {
                calculateConnectionGeometry(nativeCell, neighborCell, nodes, faceId, &faceAreaVec);
            }
            else
            {
                faceAreaVec = nativeCell.faceNormalWithAreaLenght(faceId);
            }

            if (!isFaceNormalsOutwards) faceAreaVec = -faceAreaVec;

            double halfCellTrans         = 0;
            double neighborHalfCellTrans = 0;

            // Native cell half cell transm
            {
                cvf::Vec3d centerToFace = nativeCell.faceCenter(faceId) - nativeCell.center();

                size_t permResIdx = (*permIdxFunc)(activeCellInfo, nativeResvCellIndex);
                double perm       = permResults[permResIdx];

                double ntg = 1.0;
                if (hasNTGResults && faceId != cvf::StructGridInterface::POS_K)
                {
                    size_t ntgResIdx = (*ntgIdxFunc)(activeCellInfo, nativeResvCellIndex);
                    ntg              = (*ntgResults)[ntgResIdx];
                }

                halfCellTrans = halfCellTransmissibility(perm, ntg, centerToFace, faceAreaVec);
            }

            // Neighbor cell half cell transm
            {
                cvf::Vec3d centerToFace =
                    neighborCell.faceCenter(cvf::StructGridInterface::oppositeFace(faceId)) - neighborCell.center();

                double perm = permResults[neighborCellPermResIdx];

                double ntg = 1.0;
                if (hasNTGResults && faceId != cvf::StructGridInterface::POS_K)
                {
                    size_t ntgResIdx = (*ntgIdxFunc)(activeCellInfo, neighborResvCellIdx);
                    ntg              = (*ntgResults)[ntgResIdx];
                }

                neighborHalfCellTrans = halfCellTransmissibility(perm, ntg, centerToFace, -faceAreaVec);
            }

            riTransResults[tranResIdx] = newtran(cdarchy, 1.0, halfCellTrans, neighborHalfCellTrans);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeNncCombRiTrans()
{
    size_t riCombTransScalarResultIndex =
        this->findScalarResultIndex(RiaDefines::STATIC_NATIVE, RiaDefines::combinedRiTranResultName());
    if (m_ownerMainGrid->nncData()->staticConnectionScalarResult(riCombTransScalarResultIndex)) return;

    double cdarchy = darchysValue();

    // Get the needed result indices we depend on

    size_t permXResultIdx = findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "PERMX");
    size_t permYResultIdx = findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "PERMY");
    size_t permZResultIdx = findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "PERMZ");

    size_t ntgResultIdx = findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "NTG");

    bool hasNTGResults = ntgResultIdx != cvf::UNDEFINED_SIZE_T;

    // Get all the actual result values

    std::vector<double>& permXResults = this->cellScalarResults(permXResultIdx)[0];
    std::vector<double>& permYResults = this->cellScalarResults(permYResultIdx)[0];
    std::vector<double>& permZResults = this->cellScalarResults(permZResultIdx)[0];
    std::vector<double>& riCombTransResults =
        m_ownerMainGrid->nncData()->makeStaticConnectionScalarResult(RigNNCData::propertyNameRiCombTrans());
    m_ownerMainGrid->nncData()->setScalarResultIndex(RigNNCData::propertyNameRiCombTrans(), riCombTransScalarResultIndex);

    std::vector<double>* ntgResults = nullptr;
    if (hasNTGResults)
    {
        ntgResults = &(this->cellScalarResults(ntgResultIdx)[0]);
    }

    // Prepare how to index the result values:
    ResultIndexFunction permXIdxFunc = nullptr;
    ResultIndexFunction permYIdxFunc = nullptr;
    ResultIndexFunction permZIdxFunc = nullptr;
    ResultIndexFunction ntgIdxFunc   = nullptr;
    {
        bool isPermXUsingResIdx = this->isUsingGlobalActiveIndex(permXResultIdx);
        bool isPermYUsingResIdx = this->isUsingGlobalActiveIndex(permYResultIdx);
        bool isPermZUsingResIdx = this->isUsingGlobalActiveIndex(permZResultIdx);
        bool isNtgUsingResIdx   = false;
        if (hasNTGResults)
        {
            isNtgUsingResIdx = this->isUsingGlobalActiveIndex(ntgResultIdx);
        }

        // Set up result index function pointers

        permXIdxFunc = isPermXUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;
        permYIdxFunc = isPermYUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;
        permZIdxFunc = isPermZUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;
        if (hasNTGResults)
        {
            ntgIdxFunc = isNtgUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;
        }
    }

    const RigActiveCellInfo* activeCellInfo        = this->activeCellInfo();
    bool                     isFaceNormalsOutwards = m_ownerMainGrid->isFaceNormalsOutwards();

    // NNC calculation
    std::vector<RigConnection>& nncConnections = m_ownerMainGrid->nncData()->connections();
    for (size_t connIdx = 0; connIdx < nncConnections.size(); connIdx++)
    {
        size_t                             nativeResvCellIndex = nncConnections[connIdx].m_c1GlobIdx;
        size_t                             neighborResvCellIdx = nncConnections[connIdx].m_c2GlobIdx;
        cvf::StructGridInterface::FaceType faceId              = nncConnections[connIdx].m_c1Face;

        ResultIndexFunction  permIdxFunc = nullptr;
        std::vector<double>* permResults = nullptr;

        switch (faceId)
        {
            case cvf::StructGridInterface::POS_I:
            case cvf::StructGridInterface::NEG_I:
                permIdxFunc = permXIdxFunc;
                permResults = &permXResults;
                break;
            case cvf::StructGridInterface::POS_J:
            case cvf::StructGridInterface::NEG_J:
                permIdxFunc = permYIdxFunc;
                permResults = &permYResults;
                break;
            case cvf::StructGridInterface::POS_K:
            case cvf::StructGridInterface::NEG_K:
                permIdxFunc = permZIdxFunc;
                permResults = &permZResults;
                break;
            default:
                break;
        }

        if (!permIdxFunc) continue;

        // Do nothing if we are only dealing with active cells, and this cell is not active:
        size_t nativeCellPermResIdx = (*permIdxFunc)(activeCellInfo, nativeResvCellIndex);
        if (nativeCellPermResIdx == cvf::UNDEFINED_SIZE_T) continue;

        // Do nothing if neighbor cell has no results
        size_t neighborCellPermResIdx = (*permIdxFunc)(activeCellInfo, neighborResvCellIdx);
        if (neighborCellPermResIdx == cvf::UNDEFINED_SIZE_T) continue;

        const RigCell& nativeCell   = m_ownerMainGrid->globalCellArray()[nativeResvCellIndex];
        const RigCell& neighborCell = m_ownerMainGrid->globalCellArray()[neighborResvCellIdx];

        // Connection geometry

        cvf::Vec3d faceAreaVec = cvf::Vec3d::ZERO;
        cvf::Vec3d faceCenter  = cvf::Vec3d::ZERO;

        // Polygon center
        const std::vector<cvf::Vec3d>& realPolygon = nncConnections[connIdx].m_polygon;
        for (size_t pIdx = 0; pIdx < realPolygon.size(); ++pIdx)
        {
            faceCenter += realPolygon[pIdx];
        }

        faceCenter *= 1.0 / realPolygon.size();

        // Polygon area vector

        faceAreaVec = cvf::GeometryTools::polygonAreaNormal3D(realPolygon);

        if (!isFaceNormalsOutwards) faceAreaVec = -faceAreaVec;

        double halfCellTrans         = 0;
        double neighborHalfCellTrans = 0;

        // Native cell half cell transm
        {
            cvf::Vec3d centerToFace = nativeCell.faceCenter(faceId) - nativeCell.center();

            double perm = (*permResults)[nativeCellPermResIdx];

            double ntg = 1.0;
            if (hasNTGResults && faceId != cvf::StructGridInterface::POS_K)
            {
                size_t ntgResIdx = (*ntgIdxFunc)(activeCellInfo, nativeResvCellIndex);
                ntg              = (*ntgResults)[ntgResIdx];
            }

            halfCellTrans = halfCellTransmissibility(perm, ntg, centerToFace, faceAreaVec);
        }

        // Neighbor cell half cell transm
        {
            cvf::Vec3d centerToFace =
                neighborCell.faceCenter(cvf::StructGridInterface::oppositeFace(faceId)) - neighborCell.center();

            double perm = (*permResults)[neighborCellPermResIdx];

            double ntg = 1.0;
            if (hasNTGResults && faceId != cvf::StructGridInterface::POS_K)
            {
                size_t ntgResIdx = (*ntgIdxFunc)(activeCellInfo, neighborResvCellIdx);
                ntg              = (*ntgResults)[ntgResIdx];
            }

            neighborHalfCellTrans = halfCellTransmissibility(perm, ntg, centerToFace, -faceAreaVec);
        }

        double newtranTemp          = newtran(cdarchy, 1.0, halfCellTrans, neighborHalfCellTrans);
        riCombTransResults[connIdx] = newtranTemp;
    }
}

double riMult(double transResults, double riTransResults)
{
    if (transResults == HUGE_VAL || riTransResults == HUGE_VAL) return HUGE_VAL;

    // To make 0.0 values give 1.0 in mult value
    if (cvf::Math::abs(riTransResults) < 1e-12)
    {
        if (cvf::Math::abs(transResults) < 1e-12)
        {
            return 1.0;
        }

        return HUGE_VAL;
    }

    double result = transResults / riTransResults;

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeRiMULTComponent(const QString& riMultCompName)
{
    // Set up which component to compute

    QString riTransCompName;
    QString transCompName;

    if (riMultCompName == RiaDefines::riMultXResultName())
    {
        riTransCompName = RiaDefines::riTranXResultName();
        transCompName   = "TRANX";
    }
    else if (riMultCompName == RiaDefines::riMultYResultName())
    {
        riTransCompName = RiaDefines::riTranYResultName();
        transCompName   = "TRANY";
    }
    else if (riMultCompName == RiaDefines::riMultZResultName())
    {
        riTransCompName = RiaDefines::riTranZResultName();
        transCompName   = "TRANZ";
    }
    else
    {
        CVF_ASSERT(false);
    }

    // Get the needed result indices we depend on

    size_t transResultIdx   = findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, transCompName);
    size_t riTransResultIdx = findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, riTransCompName);

    // Get the result index of the output

    size_t riMultResultIdx = this->findScalarResultIndex(RiaDefines::STATIC_NATIVE, riMultCompName);
    CVF_ASSERT(riMultResultIdx != cvf::UNDEFINED_SIZE_T);

    // Get the result count, to handle that one of them might be globally defined

    CVF_ASSERT(this->cellScalarResults(riTransResultIdx)[0].size() == this->cellScalarResults(transResultIdx)[0].size());

    size_t resultValueCount = this->cellScalarResults(transResultIdx)[0].size();

    // Get all the actual result values

    std::vector<double>& riTransResults = this->cellScalarResults(riTransResultIdx)[0];
    std::vector<double>& transResults   = this->cellScalarResults(transResultIdx)[0];

    std::vector<double>& riMultResults = this->cellScalarResults(riMultResultIdx)[0];

    // Set up output container to correct number of results

    riMultResults.resize(resultValueCount);

    for (size_t vIdx = 0; vIdx < transResults.size(); ++vIdx)
    {
        riMultResults[vIdx] = riMult(transResults[vIdx], riTransResults[vIdx]);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeNncCombRiMULT()
{
    size_t riCombMultScalarResultIndex =
        this->findScalarResultIndex(RiaDefines::STATIC_NATIVE, RiaDefines::combinedRiMultResultName());
    size_t riCombTransScalarResultIndex =
        this->findScalarResultIndex(RiaDefines::STATIC_NATIVE, RiaDefines::combinedRiTranResultName());
    size_t combTransScalarResultIndex =
        this->findScalarResultIndex(RiaDefines::STATIC_NATIVE, RiaDefines::combinedTransmissibilityResultName());

    if (m_ownerMainGrid->nncData()->staticConnectionScalarResult(riCombMultScalarResultIndex)) return;

    std::vector<double>& riMultResults =
        m_ownerMainGrid->nncData()->makeStaticConnectionScalarResult(RigNNCData::propertyNameRiCombMult());
    const std::vector<double>* riTransResults =
        m_ownerMainGrid->nncData()->staticConnectionScalarResult(riCombTransScalarResultIndex);
    const std::vector<double>* transResults =
        m_ownerMainGrid->nncData()->staticConnectionScalarResult(combTransScalarResultIndex);
    m_ownerMainGrid->nncData()->setScalarResultIndex(RigNNCData::propertyNameRiCombMult(), riCombMultScalarResultIndex);

    for (size_t nncConIdx = 0; nncConIdx < riMultResults.size(); ++nncConIdx)
    {
        riMultResults[nncConIdx] = riMult((*transResults)[nncConIdx], (*riTransResults)[nncConIdx]);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeRiTRANSbyAreaComponent(const QString& riTransByAreaCompResultName)
{
    // Set up which component to compute

    cvf::StructGridInterface::FaceType faceId = cvf::StructGridInterface::NO_FACE;
    QString                            transCompName;

    if (riTransByAreaCompResultName == RiaDefines::riAreaNormTranXResultName())
    {
        transCompName = "TRANX";
        faceId        = cvf::StructGridInterface::POS_I;
    }
    else if (riTransByAreaCompResultName == RiaDefines::riAreaNormTranYResultName())
    {
        transCompName = "TRANY";
        faceId        = cvf::StructGridInterface::POS_J;
    }
    else if (riTransByAreaCompResultName == RiaDefines::riAreaNormTranZResultName())
    {
        transCompName = "TRANZ";
        faceId        = cvf::StructGridInterface::POS_K;
    }
    else
    {
        CVF_ASSERT(false);
    }

    // Get the needed result indices we depend on

    size_t tranCompScResIdx = findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, transCompName);

    // Get the result index of the output

    size_t riTranByAreaScResIdx = this->findScalarResultIndex(RiaDefines::STATIC_NATIVE, riTransByAreaCompResultName);
    CVF_ASSERT(riTranByAreaScResIdx != cvf::UNDEFINED_SIZE_T);

    // Get the result count, to handle that one of them might be globally defined

    size_t resultValueCount = this->cellScalarResults(tranCompScResIdx)[0].size();

    // Get all the actual result values

    std::vector<double>& transResults         = this->cellScalarResults(tranCompScResIdx)[0];
    std::vector<double>& riTransByAreaResults = this->cellScalarResults(riTranByAreaScResIdx)[0];

    // Set up output container to correct number of results

    riTransByAreaResults.resize(resultValueCount);

    // Prepare how to index the result values:

    bool isUsingResIdx = this->isUsingGlobalActiveIndex(tranCompScResIdx);

    // Set up result index function pointers

    ResultIndexFunction resValIdxFunc = isUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;

    const RigActiveCellInfo*       activeCellInfo = this->activeCellInfo();
    const std::vector<cvf::Vec3d>& nodes          = m_ownerMainGrid->nodes();

    for (size_t nativeResvCellIndex = 0; nativeResvCellIndex < m_ownerMainGrid->globalCellArray().size(); nativeResvCellIndex++)
    {
        // Do nothing if we are only dealing with active cells, and this cell is not active:
        size_t nativeCellResValIdx = (*resValIdxFunc)(activeCellInfo, nativeResvCellIndex);

        if (nativeCellResValIdx == cvf::UNDEFINED_SIZE_T) continue;

        const RigCell& nativeCell = m_ownerMainGrid->globalCellArray()[nativeResvCellIndex];
        RigGridBase*   grid       = nativeCell.hostGrid();

        size_t gridLocalNativeCellIndex = nativeCell.gridLocalCellIndex();

        size_t i, j, k, gridLocalNeighborCellIdx;

        grid->ijkFromCellIndex(gridLocalNativeCellIndex, &i, &j, &k);

        if (grid->cellIJKNeighbor(i, j, k, faceId, &gridLocalNeighborCellIdx))
        {
            size_t         neighborResvCellIdx = grid->reservoirCellIndex(gridLocalNeighborCellIdx);
            const RigCell& neighborCell        = m_ownerMainGrid->globalCellArray()[neighborResvCellIdx];

            // Connection geometry

            const RigFault* fault     = grid->mainGrid()->findFaultFromCellIndexAndCellFace(nativeResvCellIndex, faceId);
            bool            isOnFault = fault;

            cvf::Vec3d faceAreaVec;

            if (isOnFault)
            {
                calculateConnectionGeometry(nativeCell, neighborCell, nodes, faceId, &faceAreaVec);
            }
            else
            {
                faceAreaVec = nativeCell.faceNormalWithAreaLenght(faceId);
            }

            double areaOfOverlap  = faceAreaVec.length();
            double transCompValue = transResults[nativeCellResValIdx];

            riTransByAreaResults[nativeCellResValIdx] = transCompValue / areaOfOverlap;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeNncCombRiTRANSbyArea()
{
    size_t riCombTransByAreaScResIdx =
        this->findScalarResultIndex(RiaDefines::STATIC_NATIVE, RiaDefines::combinedRiAreaNormTranResultName());
    size_t combTransScalarResultIndex =
        this->findScalarResultIndex(RiaDefines::STATIC_NATIVE, RiaDefines::combinedTransmissibilityResultName());

    if (m_ownerMainGrid->nncData()->staticConnectionScalarResult(riCombTransByAreaScResIdx)) return;

    std::vector<double>& riAreaNormTransResults =
        m_ownerMainGrid->nncData()->makeStaticConnectionScalarResult(RigNNCData::propertyNameRiCombTransByArea());
    m_ownerMainGrid->nncData()->setScalarResultIndex(RigNNCData::propertyNameRiCombTransByArea(), riCombTransByAreaScResIdx);
    const std::vector<double>* transResults =
        m_ownerMainGrid->nncData()->staticConnectionScalarResult(combTransScalarResultIndex);

    const std::vector<RigConnection>& connections = m_ownerMainGrid->nncData()->connections();

    for (size_t nncConIdx = 0; nncConIdx < riAreaNormTransResults.size(); ++nncConIdx)
    {
        const std::vector<cvf::Vec3d>& realPolygon   = connections[nncConIdx].m_polygon;
        cvf::Vec3d                     faceAreaVec   = cvf::GeometryTools::polygonAreaNormal3D(realPolygon);
        double                         areaOfOverlap = faceAreaVec.length();

        riAreaNormTransResults[nncConIdx] = (*transResults)[nncConIdx] / areaOfOverlap;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeCompletionTypeForTimeStep(size_t timeStep)
{
    size_t completionTypeResultIndex =
        this->findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, RiaDefines::completionTypeResultName());

    if (this->cellScalarResults(completionTypeResultIndex).size() < this->maxTimeStepCount())
    {
        this->cellScalarResults(completionTypeResultIndex).resize(this->maxTimeStepCount());
    }

    std::vector<double>& completionTypeResult = this->cellScalarResults(completionTypeResultIndex, timeStep);

    size_t resultValues = m_ownerMainGrid->globalCellArray().size();

    if (completionTypeResult.size() == resultValues)
    {
        return;
    }

    completionTypeResult.resize(resultValues);
    std::fill(completionTypeResult.begin(), completionTypeResult.end(), HUGE_VAL);

    RimEclipseCase* eclipseCase = m_ownerCaseData->ownerCase();

    if (!eclipseCase) return;

    RimCompletionCellIntersectionCalc::calculateCompletionTypeResult(eclipseCase, completionTypeResult, timeStep);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigCaseCellResultsData::darchysValue()
{
    return RiaEclipseUnitTools::darcysConstant(m_ownerCaseData->unitsType());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeCellVolumes()
{
    size_t cellVolIdx = this->findOrCreateScalarResultIndex(RiaDefines::STATIC_NATIVE, RiaDefines::riCellVolumeResultName(), false);

    if (this->cellScalarResults(cellVolIdx).empty())
    {
        this->cellScalarResults(cellVolIdx).resize(1);
    }
    std::vector<double>& cellVolumeResults = this->cellScalarResults(cellVolIdx)[0];

    size_t cellResultCount = m_activeCellInfo->reservoirCellResultCount();
    cellVolumeResults.resize(cellResultCount, std::numeric_limits<double>::infinity());

#pragma omp parallel for
    for (int nativeResvCellIndex = 0; nativeResvCellIndex < static_cast<int>(m_ownerMainGrid->globalCellArray().size()); nativeResvCellIndex++)
    {
        size_t resultIndex = activeCellInfo()->cellResultIndex(nativeResvCellIndex);
        if (resultIndex != cvf::UNDEFINED_SIZE_T)
        {
            const RigCell& cell = m_ownerMainGrid->globalCellArray()[nativeResvCellIndex];
            if (!cell.subGrid())
            {
                cellVolumeResults[resultIndex] = cell.volume();
            }
        }
    }

    // Clear oil volume so it will have to be recalculated.
    clearScalarResult(RiaDefines::DYNAMIC_NATIVE, RiaDefines::riOilVolumeResultName());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeOilVolumes()
{
    size_t cellVolIdx = this->findOrCreateScalarResultIndex(RiaDefines::STATIC_NATIVE, RiaDefines::riCellVolumeResultName(), false);
    const std::vector<double>& cellVolumeResults = this->cellScalarResults(cellVolIdx)[0];

    size_t soilIdx = this->findOrLoadScalarResult(RiaDefines::DYNAMIC_NATIVE, "SOIL");
    size_t oilVolIdx = this->findOrCreateScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, RiaDefines::riOilVolumeResultName(), false);
    this->cellScalarResults(oilVolIdx).resize(this->maxTimeStepCount());

    size_t cellResultCount = m_activeCellInfo->reservoirCellResultCount();
    for (size_t timeStepIdx = 0; timeStepIdx < this->maxTimeStepCount(); timeStepIdx++)
    {
        const std::vector<double>& soilResults = this->cellScalarResults(soilIdx)[timeStepIdx];
        std::vector<double>& oilVolumeResults = this->cellScalarResults(oilVolIdx)[timeStepIdx];
        oilVolumeResults.resize(cellResultCount, 0u);

#pragma omp parallel for
        for (int nativeResvCellIndex = 0; nativeResvCellIndex < static_cast<int>(m_ownerMainGrid->globalCellArray().size()); nativeResvCellIndex++)
        {
            size_t resultIndex = activeCellInfo()->cellResultIndex(nativeResvCellIndex);
            if (resultIndex != cvf::UNDEFINED_SIZE_T)
            {
                CVF_ASSERT(soilResults.at(resultIndex) <= 1.01);
                oilVolumeResults[resultIndex] = std::max(0.0, soilResults.at(resultIndex) * cellVolumeResults.at(resultIndex));
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeMobilePV()
{
    std::vector<double> porvDataTemp;
    std::vector<double> swcrDataTemp;
    std::vector<double> multpvDataTemp;

    const std::vector<double>* porvResults   = nullptr;
    const std::vector<double>* swcrResults   = nullptr;
    const std::vector<double>* multpvResults = nullptr;

    porvResults = RigCaseCellResultsData::getResultIndexableStaticResult(this->activeCellInfo(), this, "PORV", porvDataTemp);
    if (!porvResults || porvResults->empty())
    {
        RiaLogging::error("Assumed PORV, but not data was found.");
        return;
    }

    swcrResults = RigCaseCellResultsData::getResultIndexableStaticResult(this->activeCellInfo(), this, "SWCR", swcrDataTemp);
    multpvResults =
        RigCaseCellResultsData::getResultIndexableStaticResult(this->activeCellInfo(), this, "MULTPV", multpvDataTemp);

    size_t mobPVIdx = this->findOrCreateScalarResultIndex(RiaDefines::STATIC_NATIVE, RiaDefines::mobilePoreVolumeName(), false);

    std::vector<double>& mobPVResults = this->cellScalarResults(mobPVIdx)[0];

    // Set up output container to correct number of results
    mobPVResults.resize(porvResults->size());

    if (multpvResults && swcrResults)
    {
        for (size_t vIdx = 0; vIdx < porvResults->size(); ++vIdx)
        {
            mobPVResults[vIdx] = (*multpvResults)[vIdx] * (*porvResults)[vIdx] * (1.0 - (*swcrResults)[vIdx]);
        }
    }
    else if (!multpvResults && swcrResults)
    {
        for (size_t vIdx = 0; vIdx < porvResults->size(); ++vIdx)
        {
            mobPVResults[vIdx] = (*porvResults)[vIdx] * (1.0 - (*swcrResults)[vIdx]);
        }
    }
    else if (!swcrResults && multpvResults)
    {
        for (size_t vIdx = 0; vIdx < porvResults->size(); ++vIdx)
        {
            mobPVResults[vIdx] = (*multpvResults)[vIdx] * (*porvResults)[vIdx];
        }
    }
    else
    {
        mobPVResults.assign(porvResults->begin(), porvResults->end());
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::setReaderInterface(RifReaderInterface* readerInterface)
{
    m_readerInterface = readerInterface;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::setHdf5Filename(const QString& hdf5SourSimFilename)
{
    RifReaderEclipseOutput* rifReaderOutput = dynamic_cast<RifReaderEclipseOutput*>(m_readerInterface.p());
    if (rifReaderOutput)
    {
        rifReaderOutput->setHdf5FileName(hdf5SourSimFilename);
    }
}

//--------------------------------------------------------------------------------------------------
///  If we have any results on any time step, assume we have loaded results
//--------------------------------------------------------------------------------------------------
bool RigCaseCellResultsData::isDataPresent(size_t scalarResultIndex) const
{
    if (scalarResultIndex >= this->resultCount())
    {
        return false;
    }

    const std::vector<std::vector<double>>& data = this->cellScalarResults(scalarResultIndex);

    for (size_t tsIdx = 0; tsIdx < data.size(); ++tsIdx)
    {
        if (data[tsIdx].size())
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::assignValuesToTemporaryLgrs(const QString&       resultName,
                                                         std::vector<double>& valuesForAllReservoirCells)
{
    CVF_ASSERT(m_activeCellInfo);

    static std::vector<QString> excludedProperties = {
        "MOBPROV",  "PORV",     "FIPOIL",   "FIPGAS",   "FIPWAT",   "FLROILI+", "FLROILJ+", "FLROILK+", "FLRGASI+",
        "FLRGASJ+", "FLRGASK+", "FLRWATI+", "FLRWATJ+", "FLRWATK+", "FLOOILI+", "FLOWATI+", "FLOGASI+", "FLOOILJ+",
        "FLOWATJ+", "FLOGASJ+", "FLOOILK+", "FLOWATK+", "FLOGASK+", "SFIPGAS",  "SFIPOIL",  "SFIPWAT",  "AREAX",
        "AREAY",    "AREAZ",    "DIFFX",    "DIFFY",    "DIFFZ",    "DZNET",    "HEATTX",   "HEATTY",   "HEATTZ",
        "LX",       "LY",       "LZ",       "MINPVV",   "TRANX",    "TRANY",    "TRANZ",
    };

    if (std::find(excludedProperties.begin(), excludedProperties.end(), resultName) != excludedProperties.end())
    {
        return;
    }

    bool invalidCellsDetected = false;
    for (size_t gridIdx = 0; gridIdx < m_ownerMainGrid->gridCount(); gridIdx++)
    {
        const auto& grid = m_ownerMainGrid->gridByIndex(gridIdx);
        if (grid->isTempGrid())
        {
            for (size_t localCellIdx = 0; localCellIdx < grid->cellCount(); localCellIdx++)
            {
                const RigCell& cell = grid->cell(localCellIdx);

                size_t mainGridCellIndex  = cell.mainGridCellIndex();
                size_t reservoirCellIndex = grid->reservoirCellIndex(localCellIdx);

                size_t mainGridCellResultIndex = m_activeCellInfo->cellResultIndex(mainGridCellIndex);
                size_t cellResultIndex         = m_activeCellInfo->cellResultIndex(reservoirCellIndex);

                if (mainGridCellResultIndex != cvf::UNDEFINED_SIZE_T && cellResultIndex != cvf::UNDEFINED_SIZE_T)
                {
                    double mainGridValue = valuesForAllReservoirCells[mainGridCellResultIndex];

                    valuesForAllReservoirCells[cellResultIndex] = mainGridValue;
                }
                else
                {
                    invalidCellsDetected = true;
                }
            }
        }
    }

    if (invalidCellsDetected)
    {
        RiaLogging::warning("Detected invalid/undefined cells when assigning result values to temporary LGRs");
    }
}
