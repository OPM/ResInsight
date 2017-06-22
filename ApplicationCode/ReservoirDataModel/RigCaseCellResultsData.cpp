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

#include "RigMainGrid.h"
#include "RigStatisticsDataCache.h"
#include "RigStatisticsMath.h"
#include "RigEclipseMultiPropertyStatCalc.h"
#include "RigEclipseNativeStatCalc.h"

#include <QDateTime>
#include <math.h>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseCellResultsData::RigCaseCellResultsData(RigMainGrid* ownerGrid) : m_activeCellInfo(NULL)
{
    CVF_ASSERT(ownerGrid != NULL);
    m_ownerMainGrid = ownerGrid;
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
const std::vector< std::vector<double> > & RigCaseCellResultsData::cellScalarResults( size_t scalarResultIndex ) const
{
    CVF_TIGHT_ASSERT(scalarResultIndex < resultCount());

    return m_cellScalarResults[scalarResultIndex];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector< std::vector<double> > & RigCaseCellResultsData::cellScalarResults( size_t scalarResultIndex )
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
size_t RigCaseCellResultsData::findScalarResultIndex(RimDefines::ResultCatType type, const QString& resultName) const
{
    std::vector<ResultInfo>::const_iterator it;
    for (it = m_resultInfos.begin(); it != m_resultInfos.end(); ++it)
    {
        if (it->m_resultType == type && it->m_resultName == resultName)
        {
            return it->m_gridScalarResultIndex;
        }
    }

    return cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::findScalarResultIndex(const QString& resultName) const
{
    size_t scalarResultIndex = this->findScalarResultIndex(RimDefines::STATIC_NATIVE, resultName);

    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        scalarResultIndex = this->findScalarResultIndex(RimDefines::DYNAMIC_NATIVE, resultName);
    }

    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        scalarResultIndex = this->findScalarResultIndex(RimDefines::GENERATED, resultName);
    }

    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        scalarResultIndex = this->findScalarResultIndex(RimDefines::INPUT_PROPERTY, resultName);
    }

    if(scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        scalarResultIndex = this->findScalarResultIndex(RimDefines::FORMATION_NAMES, resultName);
    }

    return scalarResultIndex;
}

//--------------------------------------------------------------------------------------------------
/// Adds an empty scalar set, and returns the scalarResultIndex to it.
/// if resultName already exists, it just returns the scalarResultIndex to the existing result.
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::addEmptyScalarResult(RimDefines::ResultCatType type, const QString& resultName, bool needsToBeStored)
{
    size_t scalarResultIndex = this->findScalarResultIndex(type, resultName);

    // If the result exists, do nothing

    if (scalarResultIndex != cvf::UNDEFINED_SIZE_T)
    {
        return scalarResultIndex;
    }

    // Create the new empty result with metadata

    scalarResultIndex = this->resultCount();
    m_cellScalarResults.push_back(std::vector<std::vector<double> >());
    ResultInfo resInfo(type, needsToBeStored, false, resultName, scalarResultIndex);
    m_resultInfos.push_back(resInfo);

    // Create statistics calculator and add statistics cache object
    // Todo: Move to a "factory" method

    cvf::ref<RigStatisticsCalculator> statisticsCalculator;

    if (resultName == RimDefines::combinedTransmissibilityResultName())
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();

        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RimDefines::STATIC_NATIVE, "TRANX"));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RimDefines::STATIC_NATIVE, "TRANY"));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RimDefines::STATIC_NATIVE, "TRANZ"));

        statisticsCalculator = calc;
    }
    else if (resultName == RimDefines::combinedMultResultName())
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();

        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RimDefines::STATIC_NATIVE, "MULTX"));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RimDefines::STATIC_NATIVE, "MULTX-"));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RimDefines::STATIC_NATIVE, "MULTY"));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RimDefines::STATIC_NATIVE, "MULTY-"));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RimDefines::STATIC_NATIVE, "MULTZ"));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RimDefines::STATIC_NATIVE, "MULTZ-"));

        statisticsCalculator = calc;
    }
    else if (resultName == RimDefines::combinedRiTranResultName())
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RimDefines::STATIC_NATIVE, RimDefines::riTranXResultName()));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RimDefines::STATIC_NATIVE, RimDefines::riTranYResultName()));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RimDefines::STATIC_NATIVE, RimDefines::riTranZResultName()));
        statisticsCalculator = calc;
    }
    else if (resultName == RimDefines::combinedRiMultResultName())
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RimDefines::STATIC_NATIVE, RimDefines::riMultXResultName()));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RimDefines::STATIC_NATIVE, RimDefines::riMultYResultName()));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RimDefines::STATIC_NATIVE, RimDefines::riMultZResultName()));
        statisticsCalculator = calc;
    }
    else if (resultName == RimDefines::combinedRiAreaNormTranResultName())
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RimDefines::STATIC_NATIVE, RimDefines::riAreaNormTranXResultName()));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RimDefines::STATIC_NATIVE, RimDefines::riAreaNormTranYResultName()));
        calc->addNativeStatisticsCalculator(this, findScalarResultIndex(RimDefines::STATIC_NATIVE, RimDefines::riAreaNormTranZResultName()));
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
QStringList RigCaseCellResultsData::resultNames(RimDefines::ResultCatType resType) const
{
    QStringList varList;
    std::vector<ResultInfo>::const_iterator it;
    for (it = m_resultInfos.begin(); it != m_resultInfos.end(); ++it)
    {
        if (it->m_resultType == resType )
        {
            varList.push_back(it->m_resultName);
        }
    } 
    return varList;
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
    QStringList dynResVarNames = resultNames(RimDefines::DYNAMIC_NATIVE);

    bool hasFlowFluxes = true;
    hasFlowFluxes = dynResVarNames.contains("FLRWATI+");
    hasFlowFluxes = hasFlowFluxes && dynResVarNames.contains("FLROILI+") || dynResVarNames.contains("FLRGASI+");

    return hasFlowFluxes;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime RigCaseCellResultsData::timeStepDate(size_t scalarResultIndex, size_t timeStepIndex) const
{
    if (scalarResultIndex < m_resultInfos.size() && m_resultInfos[scalarResultIndex].m_timeStepDates.size() > timeStepIndex)
        return m_resultInfos[scalarResultIndex].m_timeStepDates[timeStepIndex];
    else
        return QDateTime();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RigCaseCellResultsData::timeStepDates(size_t scalarResultIndex) const
{
    if (scalarResultIndex < m_resultInfos.size())
        return m_resultInfos[scalarResultIndex].m_timeStepDates;
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
        return m_resultInfos[scalarResultIndex].m_daysSinceSimulationStart;
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
    if (scalarResultIndex < m_resultInfos.size() && m_resultInfos[scalarResultIndex].m_timeStepReportNumbers.size() > timeStepIndex)
        return m_resultInfos[scalarResultIndex].m_timeStepReportNumbers[timeStepIndex];
    else
        return -1;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<int> RigCaseCellResultsData::reportStepNumbers(size_t scalarResultIndex) const
{
    if (scalarResultIndex < m_resultInfos.size() )
        return  m_resultInfos[scalarResultIndex].m_timeStepReportNumbers;
    else
        return std::vector<int>();

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::setTimeStepDates(size_t scalarResultIndex, const std::vector<QDateTime>& dates, const std::vector<double>& daysSinceSimulationStart, const std::vector<int>& reportStepNumbers)
{
    CVF_ASSERT(scalarResultIndex < m_resultInfos.size() );

    m_resultInfos[scalarResultIndex].m_timeStepDates = dates;
    m_resultInfos[scalarResultIndex].m_daysSinceSimulationStart = daysSinceSimulationStart;
    m_resultInfos[scalarResultIndex].m_timeStepReportNumbers = reportStepNumbers;

    std::vector< std::vector<double> >& dataValues = this->cellScalarResults(scalarResultIndex);
    dataValues.resize(dates.size());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::maxTimeStepCount(size_t* scalarResultIndexWithMostTimeSteps) const
{
    size_t maxTsCount = 0;
    size_t scalarResultIndexWithMaxTsCount = cvf::UNDEFINED_SIZE_T;

    for (size_t i = 0; i < m_resultInfos.size(); i++)
    {
        if (m_resultInfos[i].m_timeStepDates.size() > maxTsCount)
        {
            maxTsCount = m_resultInfos[i].m_timeStepDates.size();
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
    size_t resultIndex = cvf::UNDEFINED_SIZE_T;
    int nameNum = 1;
    int stringLength = newResultName.size();
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
void RigCaseCellResultsData::removeResult(const QString& resultName)
{
    size_t resultIdx = findScalarResultIndex(resultName);
    if (resultIdx == cvf::UNDEFINED_SIZE_T) return;

    m_cellScalarResults[resultIdx].clear();

    m_resultInfos[resultIdx].m_resultType = RimDefines::REMOVED;
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
/// Make sure we have a result with given type and name, and make sure one "timestep" result vector 
// for the static result values are allocated
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::addStaticScalarResult(RimDefines::ResultCatType type, const QString& resultName, bool needsToBeStored, size_t resultValueCount)
{
    size_t resultIdx = addEmptyScalarResult(type, resultName, needsToBeStored);
    
    m_cellScalarResults[resultIdx].resize(1, std::vector<double>());
    m_cellScalarResults[resultIdx][0].resize(resultValueCount, HUGE_VAL);

    return resultIdx;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderInterface::PorosityModelResultType RigCaseCellResultsData::convertFromProjectModelPorosityModel(RimDefines::PorosityModelType porosityModel)
{
    if (porosityModel == RimDefines::MATRIX_MODEL) return RifReaderInterface::MATRIX_RESULTS;
    
    return RifReaderInterface::FRACTURE_RESULTS;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigCaseCellResultsData::updateResultName(RimDefines::ResultCatType resultType, QString& oldName, const QString& newName)
{
    bool anyNameUpdated = false;

    for (auto& it : m_resultInfos)
    {
        if (it.m_resultType == resultType && it.m_resultName == oldName)
        {
            anyNameUpdated = true;
            it.m_resultName = newName;
        }
    }

    return anyNameUpdated;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigCaseCellResultsData::mustBeCalculated(size_t scalarResultIndex) const
{
    std::vector<ResultInfo>::const_iterator it;
    for (it = m_resultInfos.begin(); it != m_resultInfos.end(); ++it)
    {
        if (it->m_gridScalarResultIndex == scalarResultIndex)
        {
            return it->m_mustBeCalculated;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::setMustBeCalculated(size_t scalarResultIndex)
{
    std::vector<ResultInfo>::iterator it;
    for (it = m_resultInfos.begin(); it != m_resultInfos.end(); ++it)
    {
        if (it->m_gridScalarResultIndex == scalarResultIndex)
        {
            it->m_mustBeCalculated = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::setSourSimData(size_t scalarResultIndex)
{
    CVF_ASSERT(scalarResultIndex < m_resultInfos.size());

    m_resultInfos[scalarResultIndex].m_isSourSimData = true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::eraseAllSourSimData()
{
    std::vector<size_t> sourSimIndices;

    for (size_t i = 0; i < m_resultInfos.size(); i++)
    {
        ResultInfo& ri = m_resultInfos[i];
        if (ri.m_isSourSimData)
        {
            ri.m_resultType = RimDefines::REMOVED;
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
        size_t soilIndex = findScalarResultIndex(RimDefines::DYNAMIC_NATIVE, "SOIL");
        if (soilIndex == cvf::UNDEFINED_SIZE_T)
        {
            size_t swatIndex = findScalarResultIndex(RimDefines::DYNAMIC_NATIVE, "SWAT");
            size_t sgasIndex = findScalarResultIndex(RimDefines::DYNAMIC_NATIVE, "SGAS");

            if (swatIndex != cvf::UNDEFINED_SIZE_T || sgasIndex != cvf::UNDEFINED_SIZE_T)
            {
                soilIndex = addEmptyScalarResult(RimDefines::DYNAMIC_NATIVE, "SOIL", false);
                this->setMustBeCalculated(soilIndex);
            }
        }
    }

    // TRANSXYZ
    {
        size_t tranX, tranY, tranZ;
        if (findTransmissibilityResults(tranX, tranY, tranZ))
        {
            addStaticScalarResult(RimDefines::STATIC_NATIVE, RimDefines::combinedTransmissibilityResultName(), false, 0);
        }
    }
    // MULTXYZ
    {
        addStaticScalarResult(RimDefines::STATIC_NATIVE, RimDefines::combinedMultResultName(), false, 0);
    }

    // riTRANSXYZ and X,Y,Z
    {
        if (   findScalarResultIndex(RimDefines::STATIC_NATIVE, "PERMX") != cvf::UNDEFINED_SIZE_T
            && findScalarResultIndex(RimDefines::STATIC_NATIVE, "PERMY") != cvf::UNDEFINED_SIZE_T
            && findScalarResultIndex(RimDefines::STATIC_NATIVE, "PERMZ") != cvf::UNDEFINED_SIZE_T)
        {
            addStaticScalarResult(RimDefines::STATIC_NATIVE, RimDefines::riTranXResultName(), false, 0);
            addStaticScalarResult(RimDefines::STATIC_NATIVE, RimDefines::riTranYResultName(), false, 0);
            addStaticScalarResult(RimDefines::STATIC_NATIVE, RimDefines::riTranZResultName(), false, 0);
            addStaticScalarResult(RimDefines::STATIC_NATIVE, RimDefines::combinedRiTranResultName(), false, 0);
        }
    }

    // riMULTXYZ and X, Y, Z
    {
        size_t tranX, tranY, tranZ;
        if (findTransmissibilityResults(tranX, tranY, tranZ)
            && findScalarResultIndex(RimDefines::STATIC_NATIVE, RimDefines::riTranXResultName()) != cvf::UNDEFINED_SIZE_T
            && findScalarResultIndex(RimDefines::STATIC_NATIVE, RimDefines::riTranYResultName()) != cvf::UNDEFINED_SIZE_T
            && findScalarResultIndex(RimDefines::STATIC_NATIVE, RimDefines::riTranZResultName()) != cvf::UNDEFINED_SIZE_T)
        {
            addStaticScalarResult(RimDefines::STATIC_NATIVE, RimDefines::riMultXResultName(), false, 0);
            addStaticScalarResult(RimDefines::STATIC_NATIVE, RimDefines::riMultYResultName(), false, 0);
            addStaticScalarResult(RimDefines::STATIC_NATIVE, RimDefines::riMultZResultName(), false, 0);
            addStaticScalarResult(RimDefines::STATIC_NATIVE, RimDefines::combinedRiMultResultName(), false, 0);
        }
    }

    // riTRANSXYZbyArea and X, Y, Z
    {
        if (findScalarResultIndex(RimDefines::STATIC_NATIVE, "TRANX") != cvf::UNDEFINED_SIZE_T)
        {
            addStaticScalarResult(RimDefines::STATIC_NATIVE, RimDefines::riAreaNormTranXResultName(), false, 0);
        }

        if (findScalarResultIndex(RimDefines::STATIC_NATIVE, "TRANY") != cvf::UNDEFINED_SIZE_T)
        {
            addStaticScalarResult(RimDefines::STATIC_NATIVE, RimDefines::riAreaNormTranYResultName(), false, 0);
        }

        if (findScalarResultIndex(RimDefines::STATIC_NATIVE, "TRANZ") != cvf::UNDEFINED_SIZE_T)
        {
            addStaticScalarResult(RimDefines::STATIC_NATIVE, RimDefines::riAreaNormTranZResultName(), false, 0);
        }

        size_t tranX, tranY, tranZ;
        if (findTransmissibilityResults(tranX, tranY, tranZ))
        {
            addStaticScalarResult(RimDefines::STATIC_NATIVE, RimDefines::combinedRiAreaNormTranResultName(), false, 0);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigCaseCellResultsData::findTransmissibilityResults(size_t& tranX, size_t& tranY, size_t& tranZ) const
{
    tranX = findScalarResultIndex(RimDefines::STATIC_NATIVE, "TRANX");
    tranY = findScalarResultIndex(RimDefines::STATIC_NATIVE, "TRANY");
    tranZ = findScalarResultIndex(RimDefines::STATIC_NATIVE, "TRANZ");

    if (tranX == cvf::UNDEFINED_SIZE_T ||
        tranY == cvf::UNDEFINED_SIZE_T ||
        tranZ == cvf::UNDEFINED_SIZE_T)
    {
        return false;
    }

    return true;
}

