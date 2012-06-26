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

#include "RIStdInclude.h"

#include "RigReservoirCellResults.h"
#include "RifReaderInterface.h"
#include "RigMainGrid.h"

#include <QDateTime>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigReservoirCellResults::RigReservoirCellResults(RigMainGrid* ownerGrid)
{
    CVF_ASSERT(ownerGrid != NULL);
    m_ownerMainGrid = ownerGrid;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigReservoirCellResults::minMaxCellScalarValues( size_t scalarResultIndex, double& min, double& max )
{
    min = HUGE_VAL;
    max = -HUGE_VAL;
	
    CVF_ASSERT(scalarResultIndex < resultCount());

    // Extend array and cache vars

    if (scalarResultIndex >= m_maxMinValues.size() )
    {
        m_maxMinValues.resize(scalarResultIndex+1, std::make_pair(HUGE_VAL, -HUGE_VAL));
    }

    if (m_maxMinValues[scalarResultIndex].first != HUGE_VAL)
    {
        min = m_maxMinValues[scalarResultIndex].first;
        max = m_maxMinValues[scalarResultIndex].second;

        return;
    }

    size_t i;
    for (i = 0; i < timeStepCount(scalarResultIndex); i++)
    {
        double tsmin, tsmax;
        minMaxCellScalarValues(scalarResultIndex, i, tsmin, tsmax);
        if (tsmin < min) min = tsmin;
        if (tsmax > max) max = tsmax;
    }

    m_maxMinValues[scalarResultIndex].first = min;
    m_maxMinValues[scalarResultIndex].second= max;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigReservoirCellResults::minMaxCellScalarValues(size_t scalarResultIndex, size_t timeStepIndex, double& min, double& max)
{
    min = HUGE_VAL;
    max = -HUGE_VAL;

    CVF_ASSERT(scalarResultIndex < resultCount());
    CVF_ASSERT(timeStepIndex < m_cellScalarResults[scalarResultIndex].size() );

    if (scalarResultIndex >= m_maxMinValuesPrTs.size())
    {
        m_maxMinValuesPrTs.resize(scalarResultIndex+1);
    }

    if (timeStepIndex >= m_maxMinValuesPrTs[scalarResultIndex].size())
    {
        m_maxMinValuesPrTs[scalarResultIndex].resize(timeStepIndex+1, std::make_pair(HUGE_VAL, -HUGE_VAL));
    }

    if (m_maxMinValuesPrTs[scalarResultIndex][timeStepIndex].first != HUGE_VAL)
    {
        min = m_maxMinValuesPrTs[scalarResultIndex][timeStepIndex].first;
        max = m_maxMinValuesPrTs[scalarResultIndex][timeStepIndex].second;

        return;
    }

    std::vector<double>& values = m_cellScalarResults[scalarResultIndex][timeStepIndex];

    size_t i;
    for (i = 0; i < values.size(); i++)
    {
        if (values[i] < min)
        {
            min = values[i];
        }

        if (values[i] > max)
        {
            max = values[i];
        }
    }

    m_maxMinValuesPrTs[scalarResultIndex][timeStepIndex].first = min;
    m_maxMinValuesPrTs[scalarResultIndex][timeStepIndex].second= max;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigReservoirCellResults::resultCount() const
{
	return m_cellScalarResults.size();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigReservoirCellResults::timeStepCount(size_t scalarResultIndex) const
{
    CVF_TIGHT_ASSERT(scalarResultIndex < resultCount());

    return m_cellScalarResults[scalarResultIndex].size();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector< std::vector<double> > & RigReservoirCellResults::cellScalarResults( size_t scalarResultIndex )
{
	CVF_TIGHT_ASSERT(scalarResultIndex < resultCount());

	return m_cellScalarResults[scalarResultIndex];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector< std::vector<double> >& RigReservoirCellResults::cellScalarResults( size_t scalarResultIndex ) const
{
	CVF_TIGHT_ASSERT(scalarResultIndex < resultCount());

	return m_cellScalarResults[scalarResultIndex];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigReservoirCellResults::findOrLoadScalarResult(RimDefines::ResultCatType type, const QString& resultName)
{
    size_t resultGridIndex = cvf::UNDEFINED_SIZE_T;

    resultGridIndex = findScalarResultIndex(type, resultName);

    if (resultGridIndex == cvf::UNDEFINED_SIZE_T)  return cvf::UNDEFINED_SIZE_T;

    if (cellScalarResults(resultGridIndex).size()) return resultGridIndex;

    // Generated and Input properties should always be loaded up front.
    CVF_ASSERT(type == RimDefines::STATIC_NATIVE || type == RimDefines::DYNAMIC_NATIVE);

    if (m_readerInterface.notNull())
    {
        // Add one more result to result container
        size_t timeStepCount = m_resultInfos[resultGridIndex].m_timeStepDates.size();

        bool resultLoadingSucess = true;

        if (type == RimDefines::DYNAMIC_NATIVE && timeStepCount > 0)
        {
            m_cellScalarResults[resultGridIndex].resize(timeStepCount);

            size_t i;
            for (i = 0; i < timeStepCount; i++)
            {
                std::vector<double>& values = m_cellScalarResults[resultGridIndex][i];
                if (!m_readerInterface->dynamicResult(resultName, i, &values))
                {
                    resultLoadingSucess = false;
                }
            }
        }
        else if (type == RimDefines::STATIC_NATIVE)
        {
            m_cellScalarResults[resultGridIndex].resize(1);

            std::vector<double>& values = m_cellScalarResults[resultGridIndex][0];
            if (!m_readerInterface->staticResult(resultName, &values))
            {
                resultLoadingSucess = false;
            }
        }

        if (!resultLoadingSucess)
        {
            // Remove last scalar result because loading of result failed
            m_cellScalarResults[resultGridIndex].clear();
        }
    }

    return resultGridIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigReservoirCellResults::setReaderInterface(RifReaderInterface* readerInterface)
{
    m_readerInterface = readerInterface;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigReservoirCellResults::findScalarResultIndex(RimDefines::ResultCatType type, const QString& resultName) const
{
    std::vector<ResultInfo>::const_iterator it;
    for (it = m_resultInfos.begin(); it != m_resultInfos.end(); it++)
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
size_t RigReservoirCellResults::findScalarResultIndex(const QString& resultName) const
{
    size_t scalarResultIndex = cvf::UNDEFINED_SIZE_T;

    scalarResultIndex = this->findScalarResultIndex(RimDefines::STATIC_NATIVE, resultName);

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

    return scalarResultIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigReservoirCellResults::loadOrComputeSOIL()
{
    size_t resultGridIndex = findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SOIL");

    if (resultGridIndex == cvf::UNDEFINED_SIZE_T)
    {
        size_t scalarIndexSWAT = findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SWAT");
        size_t scalarIndexSGAS = findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SGAS");

        if (scalarIndexSGAS != cvf::UNDEFINED_SIZE_T && scalarIndexSWAT != cvf::UNDEFINED_SIZE_T)
        {
            size_t timeStepCount = m_resultInfos[scalarIndexSWAT].m_timeStepDates.size();
            resultGridIndex = addEmptyScalarResult(RimDefines::DYNAMIC_NATIVE, "SOIL");
            m_cellScalarResults[resultGridIndex].resize(timeStepCount);

            const std::vector< std::vector<double> >& sgas = cellScalarResults(scalarIndexSGAS);
            const std::vector< std::vector<double> >& swat = cellScalarResults(scalarIndexSWAT);
            std::vector< std::vector<double> >& soil = cellScalarResults(resultGridIndex);

            size_t resultValueCount = sgas[0].size();

            int timeStepIdx = 0;
            for (timeStepIdx = 0; timeStepIdx < static_cast<int>(timeStepCount); timeStepIdx++)
            {
                soil[timeStepIdx].resize(resultValueCount);
                int idx = 0;
#pragma omp parallel for
                for (idx = 0; idx < static_cast<int>(resultValueCount); idx++)
                {
                    soil[timeStepIdx][idx] = 1.0 - sgas[timeStepIdx][idx] - swat[timeStepIdx][idx];
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigReservoirCellResults::findOrLoadScalarResult(const QString& resultName)
{
    size_t scalarResultIndex = cvf::UNDEFINED_SIZE_T;

    scalarResultIndex = this->findOrLoadScalarResult(RimDefines::STATIC_NATIVE, resultName);
    
    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        scalarResultIndex = this->findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, resultName);
    }

    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        scalarResultIndex = this->findScalarResultIndex(RimDefines::GENERATED, resultName);
    }

    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        scalarResultIndex = this->findScalarResultIndex(RimDefines::INPUT_PROPERTY, resultName);
    }

    return scalarResultIndex;
}

//--------------------------------------------------------------------------------------------------
/// Adds an empty scalar set, and returns the scalarResultIndex to it.
/// if resultName already exists, it returns the scalarResultIndex to the existing result.
//--------------------------------------------------------------------------------------------------
size_t RigReservoirCellResults::addEmptyScalarResult(RimDefines::ResultCatType type, const QString& resultName)
{
    size_t scalarResultIndex = cvf::UNDEFINED_SIZE_T;

    scalarResultIndex = this->findScalarResultIndex(type, resultName);
    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        scalarResultIndex = this->resultCount();
        m_cellScalarResults.push_back(std::vector<std::vector<double> >());
        ResultInfo resInfo(type, resultName, scalarResultIndex);
        m_resultInfos.push_back(resInfo);
    }

    return scalarResultIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RigReservoirCellResults::resultNames(RimDefines::ResultCatType resType) const
{
    QStringList varList;
    std::vector<ResultInfo>::const_iterator it;
    for (it = m_resultInfos.begin(); it != m_resultInfos.end(); it++)
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
void RigReservoirCellResults::recalculateMinMax(size_t scalarResultIndex)
{
    // Make sure cached max min values are recalculated next time asked for, since
    // the data could be changed.

    if (scalarResultIndex < m_maxMinValues.size())
    {
        m_maxMinValues[scalarResultIndex] = std::make_pair(HUGE_VAL, -HUGE_VAL);
    }

    if (scalarResultIndex < m_maxMinValuesPrTs.size())
    {
        m_maxMinValuesPrTs[scalarResultIndex].clear();
    }
}

//--------------------------------------------------------------------------------------------------
/// Returns whether the result data in question is addressed by Active Cell Index
//--------------------------------------------------------------------------------------------------
bool RigReservoirCellResults::isUsingGlobalActiveIndex(size_t scalarResultIndex) const
{
    CVF_TIGHT_ASSERT(scalarResultIndex < m_cellScalarResults.size());

    if (!m_cellScalarResults[scalarResultIndex].size()) return true;
    if (m_cellScalarResults[scalarResultIndex][0].size() == m_ownerMainGrid->numActiveCells()) return true;
    if (m_cellScalarResults[scalarResultIndex][0].size() == m_ownerMainGrid->cellCount()) return false;

    CVF_TIGHT_ASSERT(false); // Wrong number of results

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<QDateTime> RigReservoirCellResults::timeStepDates(size_t scalarResultIndex) const
{
    if (scalarResultIndex < m_resultInfos.size() )
        return  m_resultInfos[scalarResultIndex].m_timeStepDates;
    else
        return QList<QDateTime>();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigReservoirCellResults::setTimeStepDates(size_t scalarResultIndex, const QList<QDateTime>& dates)
{
    CVF_ASSERT(scalarResultIndex < m_resultInfos.size() );

    m_resultInfos[scalarResultIndex].m_timeStepDates = dates;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigReservoirCellResults::maxTimeStepCount() const
{
    size_t maxTsCount = 0;
    for (size_t i = 0; i < m_cellScalarResults.size(); ++i)
    {
       maxTsCount =  m_cellScalarResults[i].size() > maxTsCount ? m_cellScalarResults[i].size() : maxTsCount;
    }
    return maxTsCount;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RigReservoirCellResults::makeResultNameUnique(const QString& resultNameProposal) const
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
void RigReservoirCellResults::removeResult(const QString& resultName)
{
    size_t resultIdx = findScalarResultIndex(resultName);
    if (resultIdx == cvf::UNDEFINED_SIZE_T) return;

    m_cellScalarResults[resultIdx].clear();

    m_resultInfos[resultIdx].m_resultType = RimDefines::REMOVED;
}
