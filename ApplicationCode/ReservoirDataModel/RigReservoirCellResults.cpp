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

#include "RigReservoirCellResults.h"
#include "RifReaderInterface.h"
#include "RigMainGrid.h"

#include <QDateTime>
#include <math.h>


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
        if (values[i] == HUGE_VAL)
        {
            continue;
        }

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
const std::vector<size_t>& RigReservoirCellResults::cellScalarValuesHistogram(size_t scalarResultIndex)
{
    CVF_ASSERT(scalarResultIndex < resultCount());

    // Extend array and cache vars

    if (scalarResultIndex >= m_histograms.size() )
    {
        m_histograms.resize(resultCount());
        m_p10p90.resize(resultCount(), std::make_pair(HUGE_VAL, HUGE_VAL));
    }

    if (m_histograms[scalarResultIndex].size())
    {
        return m_histograms[scalarResultIndex];

    }

    double min;
    double max;
    size_t nBins = 100;
    this->minMaxCellScalarValues( scalarResultIndex, min, max );
    RigHistogramCalculator histCalc(min, max, nBins, &m_histograms[scalarResultIndex]);

    for (size_t tsIdx = 0; tsIdx < this->timeStepCount(scalarResultIndex); tsIdx++)
    {
        std::vector<double>& values = m_cellScalarResults[scalarResultIndex][tsIdx];

        histCalc.addData(values);
    } 

    m_p10p90[scalarResultIndex].first = histCalc.calculatePercentil(0.1);
    m_p10p90[scalarResultIndex].second = histCalc.calculatePercentil(0.9);

    return m_histograms[scalarResultIndex];
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigReservoirCellResults::p10p90CellScalarValues(size_t scalarResultIndex, double& p10, double& p90)
{
    const std::vector<size_t>& histogr = cellScalarValuesHistogram( scalarResultIndex);
    p10 = m_p10p90[scalarResultIndex].first;
    p90 = m_p10p90[scalarResultIndex].second;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigReservoirCellResults::meanCellScalarValues(size_t scalarResultIndex, double& meanValue)
{
    CVF_ASSERT(scalarResultIndex < resultCount());

    // Extend array and cache vars

    if (scalarResultIndex >= m_meanValues.size() )
    {
        m_meanValues.resize(scalarResultIndex+1, HUGE_VAL);
    }

    if (m_meanValues[scalarResultIndex] != HUGE_VAL)
    {
        meanValue = m_meanValues[scalarResultIndex];
        return;
    }

    double valueSum = 0.0;
    size_t count = 0;
    for (size_t tIdx = 0; tIdx < timeStepCount(scalarResultIndex); tIdx++)
    {
        std::vector<double>& values = m_cellScalarResults[scalarResultIndex][tIdx];
        for (size_t cIdx = 0; cIdx < values.size(); ++cIdx)
        {
            valueSum += values[cIdx];
        }
        count += values.size();
    }

    m_meanValues[scalarResultIndex] = valueSum/count;
    meanValue = m_meanValues[scalarResultIndex];
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
const std::vector< std::vector<double> > & RigReservoirCellResults::cellScalarResults( size_t scalarResultIndex ) const
{
    CVF_TIGHT_ASSERT(scalarResultIndex < resultCount());

    return m_cellScalarResults[scalarResultIndex];
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
double RigReservoirCellResults::cellScalarResult( size_t scalarResultIndex, size_t timeStepIndex, size_t resultValueIndex)
{
    if (scalarResultIndex < resultCount() &&
        timeStepIndex < m_cellScalarResults[scalarResultIndex].size() &&
        resultValueIndex != cvf::UNDEFINED_SIZE_T &&
        resultValueIndex < m_cellScalarResults[scalarResultIndex][timeStepIndex].size())
    {
        return m_cellScalarResults[scalarResultIndex][timeStepIndex][resultValueIndex];
    }
    else
    {
        return HUGE_VAL;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigReservoirCellResults::findOrLoadScalarResultForTimeStep(RimDefines::ResultCatType type, const QString& resultName, size_t timeStepIndex)
{
    size_t resultGridIndex = cvf::UNDEFINED_SIZE_T;

    resultGridIndex = findScalarResultIndex(type, resultName);

    if (resultGridIndex == cvf::UNDEFINED_SIZE_T)  return cvf::UNDEFINED_SIZE_T;

    if (type == RimDefines::GENERATED)
    {
        return cvf::UNDEFINED_SIZE_T;
    }

    if (m_readerInterface.notNull())
    {
        // Add one more result to result container
        size_t timeStepCount = m_resultInfos[resultGridIndex].m_timeStepDates.size();

        bool resultLoadingSucess = true;

        if (type == RimDefines::DYNAMIC_NATIVE && timeStepCount > 0)
        {
            std::vector<double>& values = m_cellScalarResults[resultGridIndex][timeStepIndex];
            if (values.size() == 0)
            {
                if (!m_readerInterface->dynamicResult(resultName, RifReaderInterface::MATRIX_RESULTS, timeStepIndex, &values))
                {
                    resultLoadingSucess = false;
                }
            }
        }
        else if (type == RimDefines::STATIC_NATIVE)
        {
            std::vector<double>& values = m_cellScalarResults[resultGridIndex][0];
            if (!m_readerInterface->staticResult(resultName, RifReaderInterface::MATRIX_RESULTS, &values))
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

    return resultGridIndex;

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

    if (type == RimDefines::GENERATED)
    {
        return cvf::UNDEFINED_SIZE_T;
    }

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
                if (!m_readerInterface->dynamicResult(resultName, RifReaderInterface::MATRIX_RESULTS, i, &values))
                {
                    resultLoadingSucess = false;
                }
            }
        }
        else if (type == RimDefines::STATIC_NATIVE)
        {
            m_cellScalarResults[resultGridIndex].resize(1);

            std::vector<double>& values = m_cellScalarResults[resultGridIndex][0];
            if (!m_readerInterface->staticResult(resultName, RifReaderInterface::MATRIX_RESULTS, &values))
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
RifReaderInterface* RigReservoirCellResults::readerInterface()
{
    return m_readerInterface.p();
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
    size_t soilResultGridIndex = findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SOIL");

    if (soilResultGridIndex == cvf::UNDEFINED_SIZE_T)
    {
        size_t scalarIndexSWAT = findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SWAT");
        size_t scalarIndexSGAS = findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SGAS");

        // Early exit if none of SWAT or SGAS is present
        if (scalarIndexSWAT == cvf::UNDEFINED_SIZE_T && scalarIndexSGAS == cvf::UNDEFINED_SIZE_T)
        {
            return;
        }

        soilResultGridIndex = addEmptyScalarResult(RimDefines::DYNAMIC_NATIVE, "SOIL");

        const std::vector< std::vector<double> >* swat = NULL;
        const std::vector< std::vector<double> >* sgas = NULL;
        if (scalarIndexSWAT != cvf::UNDEFINED_SIZE_T)
        {
            swat = &(cellScalarResults(scalarIndexSWAT));
        }

        if (scalarIndexSGAS != cvf::UNDEFINED_SIZE_T)
        {
            sgas = &(cellScalarResults(scalarIndexSGAS));
        }

        size_t soilResultValueCount = 0;
        size_t soilTimeStepCount = 0;
        if (swat)
        {
            soilResultValueCount = swat->at(0).size();
            soilTimeStepCount = m_resultInfos[scalarIndexSWAT].m_timeStepDates.size();
        }

        if (sgas)
        {
            soilResultValueCount = qMax(soilResultValueCount, sgas->at(0).size());
            
            size_t sgasTimeStepCount = m_resultInfos[scalarIndexSGAS].m_timeStepDates.size();
            soilTimeStepCount = qMax(soilTimeStepCount, sgasTimeStepCount);
        }

        m_cellScalarResults[soilResultGridIndex].resize(soilTimeStepCount);

        std::vector< std::vector<double> >& soil = cellScalarResults(soilResultGridIndex);

        int timeStepIdx = 0;
        for (timeStepIdx = 0; timeStepIdx < static_cast<int>(soilTimeStepCount); timeStepIdx++)
        {
            soil[timeStepIdx].resize(soilResultValueCount);

#pragma omp parallel for
            for (int idx = 0; idx < static_cast<int>(soilResultValueCount); idx++)
            {
                double soilValue = 1.0;
                if (sgas)
                {
                    soilValue -= sgas->at(timeStepIdx)[idx];
                }

                if (swat)
                {
                    soilValue -= swat->at(timeStepIdx)[idx];
                }

                soil[timeStepIdx][idx] = soilValue;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigReservoirCellResults::computeDepthRelatedResults()
{
    size_t depthResultGridIndex  = findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "DEPTH");
    size_t dxResultGridIndex     = findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "DX");
    size_t dyResultGridIndex     = findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "DY");
    size_t dzResultGridIndex     = findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "DZ");
    size_t topsResultGridIndex   = findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "TOPS");
    size_t bottomResultGridIndex = findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "BOTTOM");

    bool computeDepth = false;
    bool computeDx = false;
    bool computeDy = false;
    bool computeDz = false;
    bool computeTops = false;
    bool computeBottom = false;

    size_t resultValueCount = m_ownerMainGrid->cells().size();

    if (depthResultGridIndex == cvf::UNDEFINED_SIZE_T)
    {
        depthResultGridIndex = addStaticScalarResult(RimDefines::STATIC_NATIVE, "DEPTH", resultValueCount);
        computeDepth = true;
    }

    if (dxResultGridIndex == cvf::UNDEFINED_SIZE_T)
    {
        dxResultGridIndex = addStaticScalarResult(RimDefines::STATIC_NATIVE, "DX", resultValueCount);
        computeDx = true;
    }

    if (dyResultGridIndex == cvf::UNDEFINED_SIZE_T)
    {
        dyResultGridIndex = addStaticScalarResult(RimDefines::STATIC_NATIVE, "DY", resultValueCount);
        computeDy = true;
    }
    
    if (dzResultGridIndex == cvf::UNDEFINED_SIZE_T)
    {
        dzResultGridIndex = addStaticScalarResult(RimDefines::STATIC_NATIVE, "DZ", resultValueCount);
        computeDz = true;
    }
    
    if (topsResultGridIndex == cvf::UNDEFINED_SIZE_T)
    {
        topsResultGridIndex = addStaticScalarResult(RimDefines::STATIC_NATIVE, "TOPS", resultValueCount);
        computeTops = true;
    }
    
    if (bottomResultGridIndex == cvf::UNDEFINED_SIZE_T)
    {
        bottomResultGridIndex = addStaticScalarResult(RimDefines::STATIC_NATIVE, "BOTTOM", resultValueCount);
        computeBottom = true;
    }

    std::vector< std::vector<double> >& depth   = cellScalarResults(depthResultGridIndex);
    std::vector< std::vector<double> >& dx      = cellScalarResults(dxResultGridIndex);
    std::vector< std::vector<double> >& dy      = cellScalarResults(dyResultGridIndex);
    std::vector< std::vector<double> >& dz      = cellScalarResults(dzResultGridIndex);
    std::vector< std::vector<double> >& tops    = cellScalarResults(topsResultGridIndex);
    std::vector< std::vector<double> >& bottom  = cellScalarResults(bottomResultGridIndex);
    
    size_t cellIdx = 0;
    for (cellIdx = 0; cellIdx < m_ownerMainGrid->cells().size(); cellIdx++)
    {
        const RigCell& cell = m_ownerMainGrid->cells()[cellIdx];

        if (computeDepth)
        {
            depth[0][cellIdx] = cvf::Math::abs(cell.center().z());
        }

        if (computeDx)
        {
            cvf::Vec3d cellWidth = cell.faceCenter(cvf::StructGridInterface::NEG_I) - cell.faceCenter(cvf::StructGridInterface::POS_I);
            dx[0][cellIdx] =  cvf::Math::abs(cellWidth.x());
        }

        if (computeDy)
        {
            cvf::Vec3d cellWidth = cell.faceCenter(cvf::StructGridInterface::NEG_J) - cell.faceCenter(cvf::StructGridInterface::POS_J);
            dy[0][cellIdx] =  cvf::Math::abs(cellWidth.y());
        }

        if (computeDz)
        {
            cvf::Vec3d cellWidth = cell.faceCenter(cvf::StructGridInterface::NEG_K) - cell.faceCenter(cvf::StructGridInterface::POS_K);
            dz[0][cellIdx] = cvf::Math::abs(cellWidth.z());
        }

        if (computeTops)
        {
            tops[0][cellIdx] = cvf::Math::abs(cell.faceCenter(cvf::StructGridInterface::NEG_K).z());
        }

        if (computeBottom)
        {
            bottom[0][cellIdx] = cvf::Math::abs(cell.faceCenter(cvf::StructGridInterface::POS_K).z());
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
    
    size_t firstTimeStepResultValueCount = m_cellScalarResults[scalarResultIndex][0].size();
    if (firstTimeStepResultValueCount == m_ownerMainGrid->cells().size()) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime RigReservoirCellResults::timeStepDate(size_t scalarResultIndex, size_t timeStepIndex) const
{
    if (scalarResultIndex < m_resultInfos.size() && (size_t)(m_resultInfos[scalarResultIndex].m_timeStepDates.size()) > timeStepIndex)
        return m_resultInfos[scalarResultIndex].m_timeStepDates[static_cast<int>(timeStepIndex)];
    else
        return QDateTime();
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
    int maxTsCount = 0;

    std::vector<ResultInfo>::const_iterator it;
    for (it = m_resultInfos.begin(); it != m_resultInfos.end(); it++)
    {
        maxTsCount = it->m_timeStepDates.size() > maxTsCount ? it->m_timeStepDates.size() : maxTsCount;
    } 

    return static_cast<size_t>(maxTsCount);
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigReservoirCellResults::clearAllResults()
{
    for (size_t i = 0; i < m_cellScalarResults.size(); i++)
    {
        m_cellScalarResults[i].clear();
    }
}

//--------------------------------------------------------------------------------------------------
/// Add a result with given type and name, and allocate one result vector for the static result values
//--------------------------------------------------------------------------------------------------
size_t RigReservoirCellResults::addStaticScalarResult(RimDefines::ResultCatType type, const QString& resultName, size_t resultValueCount)
{
    size_t resultIdx = addEmptyScalarResult(type, resultName);
    
    m_cellScalarResults[resultIdx].push_back(std::vector<double>());
    m_cellScalarResults[resultIdx][0].resize(resultValueCount, HUGE_VAL);

    return resultIdx;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderInterface::PorosityModelResultType RigReservoirCellResults::convertFromProjectModelPorosityModel(RimDefines::PorosityModelType porosityModel)
{
    if (porosityModel == RimDefines::MATRIX_MODEL) return RifReaderInterface::MATRIX_RESULTS;
    
    return RifReaderInterface::FRACTURE_RESULTS;
}

