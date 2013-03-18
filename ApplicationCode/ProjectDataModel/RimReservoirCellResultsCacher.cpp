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
#include "RimReservoirCellResultsCacher.h"
#include "RigReservoirCellResults.h"
#include "RIApplication.h"
#include "RigMainGrid.h"
#include "RigCell.h"

CAF_PDM_SOURCE_INIT(RimReservoirCellResultsCacher, "ReservoirCellResultCacher");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoirCellResultsCacher::RimReservoirCellResultsCacher()
    : m_cellResults(NULL), 
      m_ownerMainGrid(NULL)
{
    CAF_PDM_InitObject("Cacher", "", "", "");

    CAF_PDM_InitField(&m_resultCacheFileName, "ResultCacheFileName",  QString(), "UiDummyname", "", "" ,"");
    m_resultCacheFileName.setUiHidden(true);
    CAF_PDM_InitFieldNoDefault(&m_resultCacheMetaData, "ResultCacheEntries", "UiDummyname", "", "", "");
    m_resultCacheMetaData.setUiHidden(true);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoirCellResultsCacher::~RimReservoirCellResultsCacher()
{
    m_resultCacheMetaData.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirCellResultsCacher::setupBeforeSave()
{
    if (!m_cellResults) return;

    const std::vector<RigReservoirCellResults::ResultInfo>&  resInfo = m_cellResults->infoForEachResultIndex();
    m_resultCacheMetaData.deleteAllChildObjects();

    if(resInfo.size())
    {
        QDir::root().mkpath(getCacheDirectoryPath());

        QString newValidCacheFileName = getValidCacheFileName();
        QFile cacheFile(newValidCacheFileName);

        if (!cacheFile.open(QIODevice::WriteOnly)) qWarning() << "Saving project: Can't open the cache file : " + newValidCacheFileName; 

        QDataStream stream(&cacheFile);
        stream.setVersion(QDataStream::Qt_4_0);
        stream << (quint32)0xCEECAC4E; // magic number
        stream << (qint32)1; // Version

        for (int rIdx = 0; rIdx < resInfo.size(); ++rIdx)
        {
            size_t timestepCount = m_cellResults->cellScalarResults(resInfo[rIdx].m_gridScalarResultIndex).size();
            if (timestepCount)
            {
                RimReservoirCellResultsCacheEntryInfo*  cacheEntry = new RimReservoirCellResultsCacheEntryInfo;
                m_resultCacheMetaData.push_back(cacheEntry);

                cacheEntry->m_resultType = resInfo[rIdx].m_resultType;
                cacheEntry->m_resultName = resInfo[rIdx].m_resultName;
                cacheEntry->m_timeStepDates = resInfo[rIdx].m_timeStepDates;

                cacheEntry->m_filePosition = cacheFile.pos();

                for (int tsIdx = 0; tsIdx < resInfo[rIdx].m_timeStepDates.size() ; ++tsIdx)
                {
                    const std::vector<double>* data = NULL;
                    if (tsIdx < timestepCount)
                    {
                        data = &(m_cellResults->cellScalarResults(resInfo[rIdx].m_gridScalarResultIndex, tsIdx));
                    }

                    if (data && data->size())
                    {
                        cacheEntry->m_timeStepHasData.v().push_back(1);

                        for (size_t cIdx = 0; cIdx < data->size(); ++cIdx)
                        {
                            stream << (*data)[cIdx];
                        }
                    }
                    else
                    {
                        cacheEntry->m_timeStepHasData.v().push_back(0);
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimReservoirCellResultsCacher::getValidCacheFileName()
{
    QString cacheFileName;
    if (m_resultCacheFileName().isEmpty())
    {
        QString newCacheDirPath =  getCacheDirectoryPath();
        QUuid guid = QUuid::createUuid();
        cacheFileName = newCacheDirPath + "/" + guid.toString();
    }
    else
    {
        // Make the path correct related to the possibly new project filename
        QString newCacheDirPath =  getCacheDirectoryPath();
        QFileInfo oldCacheFile(m_resultCacheFileName());

        cacheFileName = newCacheDirPath + "/" + oldCacheFile.fileName();
    }
    return cacheFileName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimReservoirCellResultsCacher::getCacheDirectoryPath()
{
    QString cacheDirPath;
    QString projectFileName = RIApplication::instance()->project()->fileName();
    QFileInfo fileInfo(projectFileName);
    cacheDirPath = fileInfo.canonicalPath();
    cacheDirPath += "/" + fileInfo.completeBaseName() + "_cache";
    return cacheDirPath;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirCellResultsCacher::setReaderInterface(RifReaderInterface* readerInterface)
{
    m_readerInterface = readerInterface;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderInterface* RimReservoirCellResultsCacher::readerInterface()
{
    return m_readerInterface.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimReservoirCellResultsCacher::findOrLoadScalarResultForTimeStep(RimDefines::ResultCatType type, const QString& resultName, size_t timeStepIndex)
{
    // Special handling for SOIL
    if (type == RimDefines::DYNAMIC_NATIVE && resultName.toUpper() == "SOIL")
    {
        loadOrComputeSOILForTimeStep(timeStepIndex);
    }

    size_t scalarResultIndex = cvf::UNDEFINED_SIZE_T;

    scalarResultIndex = m_cellResults->findScalarResultIndex(type, resultName);

    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)  return cvf::UNDEFINED_SIZE_T;

    if (type == RimDefines::GENERATED)
    {
        return cvf::UNDEFINED_SIZE_T;
    }

    if (m_readerInterface.notNull())
    {
        size_t timeStepCount = m_cellResults->infoForEachResultIndex()[scalarResultIndex].m_timeStepDates.size();

        bool resultLoadingSucess = true;

        if (type == RimDefines::DYNAMIC_NATIVE && timeStepCount > 0)
        {
            m_cellResults->cellScalarResults(scalarResultIndex).resize(timeStepCount);

            std::vector<double>& values = m_cellResults->cellScalarResults(scalarResultIndex)[timeStepIndex];
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
            m_cellResults->cellScalarResults(scalarResultIndex).resize(1);

            std::vector<double>& values = m_cellResults->cellScalarResults(scalarResultIndex)[0];
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

    return scalarResultIndex;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimReservoirCellResultsCacher::findOrLoadScalarResult(RimDefines::ResultCatType type, const QString& resultName)
{
    size_t resultGridIndex = cvf::UNDEFINED_SIZE_T;

    resultGridIndex = m_cellResults->findScalarResultIndex(type, resultName);

    if (resultGridIndex == cvf::UNDEFINED_SIZE_T)  return cvf::UNDEFINED_SIZE_T;

    if (m_cellResults->cellScalarResults(resultGridIndex).size()) return resultGridIndex;

    if (type == RimDefines::GENERATED)
    {
        return cvf::UNDEFINED_SIZE_T;
    }

    if (m_readerInterface.notNull())
    {
        // Add one more result to result container
        size_t timeStepCount = m_cellResults->infoForEachResultIndex()[resultGridIndex].m_timeStepDates.size();

        bool resultLoadingSucess = true;

        if (type == RimDefines::DYNAMIC_NATIVE && timeStepCount > 0)
        {
            m_cellResults->cellScalarResults(resultGridIndex).resize(timeStepCount);

            size_t i;
            for (i = 0; i < timeStepCount; i++)
            {
                std::vector<double>& values = m_cellResults->cellScalarResults(resultGridIndex)[i];
                if (!m_readerInterface->dynamicResult(resultName, RifReaderInterface::MATRIX_RESULTS, i, &values))
                {
                    resultLoadingSucess = false;
                }
            }
        }
        else if (type == RimDefines::STATIC_NATIVE)
        {
            m_cellResults->cellScalarResults(resultGridIndex).resize(1);

            std::vector<double>& values = m_cellResults->cellScalarResults(resultGridIndex)[0];
            if (!m_readerInterface->staticResult(resultName, RifReaderInterface::MATRIX_RESULTS, &values))
            {
                resultLoadingSucess = false;
            }
        }

        if (!resultLoadingSucess)
        {
            // Remove last scalar result because loading of result failed
            m_cellResults->cellScalarResults(resultGridIndex).clear();
        }
    }

    return resultGridIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirCellResultsCacher::loadOrComputeSOIL()
{
    for (size_t timeStepIdx = 0; timeStepIdx < m_cellResults->maxTimeStepCount(); timeStepIdx++)
    {
        loadOrComputeSOILForTimeStep(timeStepIdx);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirCellResultsCacher::loadOrComputeSOILForTimeStep(size_t timeStepIndex)
{
    size_t scalarIndexSWAT = findOrLoadScalarResultForTimeStep(RimDefines::DYNAMIC_NATIVE, "SWAT", timeStepIndex);
    size_t scalarIndexSGAS = findOrLoadScalarResultForTimeStep(RimDefines::DYNAMIC_NATIVE, "SGAS", timeStepIndex);

    // Early exit if none of SWAT or SGAS is present
    if (scalarIndexSWAT == cvf::UNDEFINED_SIZE_T && scalarIndexSGAS == cvf::UNDEFINED_SIZE_T)
    {
        return;
    }

    size_t soilResultValueCount = 0;
    size_t soilTimeStepCount = 0;

    std::vector<double>* swatForTimeStep = NULL;
    std::vector<double>* sgasForTimeStep = NULL;
    if (scalarIndexSWAT != cvf::UNDEFINED_SIZE_T)
    {
        swatForTimeStep = &(m_cellResults->cellScalarResults(scalarIndexSWAT, timeStepIndex));
        if (swatForTimeStep->size() == 0)
        {
            swatForTimeStep = NULL;
        }
        else
        {
            soilResultValueCount = swatForTimeStep->size();
            soilTimeStepCount = m_cellResults->infoForEachResultIndex()[scalarIndexSWAT].m_timeStepDates.size();
        }
    }

    if (scalarIndexSGAS != cvf::UNDEFINED_SIZE_T)
    {
        sgasForTimeStep = &(m_cellResults->cellScalarResults(scalarIndexSGAS, timeStepIndex));
        if (sgasForTimeStep->size() == 0)
        {
            sgasForTimeStep = NULL;
        }
        else
        {
            soilResultValueCount = qMax(soilResultValueCount, sgasForTimeStep->size());

            size_t sgasTimeStepCount = m_cellResults->infoForEachResultIndex()[scalarIndexSGAS].m_timeStepDates.size();
            soilTimeStepCount = qMax(soilTimeStepCount, sgasTimeStepCount);
        }
    }

    size_t soilResultGridIndex = findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SOIL");
    if (soilResultGridIndex == cvf::UNDEFINED_SIZE_T)
    {
        soilResultGridIndex = m_cellResults->addEmptyScalarResult(RimDefines::DYNAMIC_NATIVE, "SOIL");
        CVF_ASSERT(soilResultGridIndex != cvf::UNDEFINED_SIZE_T);

        m_cellResults->cellScalarResults(soilResultGridIndex).resize(soilTimeStepCount);

        for (size_t timeStepIdx = 0; timeStepIdx < soilTimeStepCount; timeStepIdx++)
        {
            m_cellResults->cellScalarResults(soilResultGridIndex, timeStepIdx).resize(soilResultValueCount);
        }
    }

    std::vector<double>& soilForTimeStep = m_cellResults->cellScalarResults(soilResultGridIndex, timeStepIndex);

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

        soilForTimeStep[idx] = soilValue;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirCellResultsCacher::computeDepthRelatedResults()
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
        depthResultGridIndex = m_cellResults->addStaticScalarResult(RimDefines::STATIC_NATIVE, "DEPTH", resultValueCount);
        computeDepth = true;
    }

    if (dxResultGridIndex == cvf::UNDEFINED_SIZE_T)
    {
        dxResultGridIndex = m_cellResults->addStaticScalarResult(RimDefines::STATIC_NATIVE, "DX", resultValueCount);
        computeDx = true;
    }

    if (dyResultGridIndex == cvf::UNDEFINED_SIZE_T)
    {
        dyResultGridIndex = m_cellResults->addStaticScalarResult(RimDefines::STATIC_NATIVE, "DY", resultValueCount);
        computeDy = true;
    }

    if (dzResultGridIndex == cvf::UNDEFINED_SIZE_T)
    {
        dzResultGridIndex = m_cellResults->addStaticScalarResult(RimDefines::STATIC_NATIVE, "DZ", resultValueCount);
        computeDz = true;
    }

    if (topsResultGridIndex == cvf::UNDEFINED_SIZE_T)
    {
        topsResultGridIndex = m_cellResults->addStaticScalarResult(RimDefines::STATIC_NATIVE, "TOPS", resultValueCount);
        computeTops = true;
    }

    if (bottomResultGridIndex == cvf::UNDEFINED_SIZE_T)
    {
        bottomResultGridIndex = m_cellResults->addStaticScalarResult(RimDefines::STATIC_NATIVE, "BOTTOM", resultValueCount);
        computeBottom = true;
    }

    std::vector< std::vector<double> >& depth   = m_cellResults->cellScalarResults(depthResultGridIndex);
    std::vector< std::vector<double> >& dx      = m_cellResults->cellScalarResults(dxResultGridIndex);
    std::vector< std::vector<double> >& dy      = m_cellResults->cellScalarResults(dyResultGridIndex);
    std::vector< std::vector<double> >& dz      = m_cellResults->cellScalarResults(dzResultGridIndex);
    std::vector< std::vector<double> >& tops    = m_cellResults->cellScalarResults(topsResultGridIndex);
    std::vector< std::vector<double> >& bottom  = m_cellResults->cellScalarResults(bottomResultGridIndex);

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
size_t RimReservoirCellResultsCacher::findOrLoadScalarResult(const QString& resultName)
{
    size_t scalarResultIndex = cvf::UNDEFINED_SIZE_T;

    scalarResultIndex = this->findOrLoadScalarResult(RimDefines::STATIC_NATIVE, resultName);

    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        scalarResultIndex = this->findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, resultName);
    }

    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        scalarResultIndex = m_cellResults->findScalarResultIndex(RimDefines::GENERATED, resultName);
    }

    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        scalarResultIndex = m_cellResults->findScalarResultIndex(RimDefines::INPUT_PROPERTY, resultName);
    }

    return scalarResultIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirCellResultsCacher::setCellResults(RigReservoirCellResults* cellResults)
{
    m_cellResults = cellResults;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirCellResultsCacher::setMainGrid(RigMainGrid* mainGrid)
{
    m_ownerMainGrid = mainGrid;
}


CAF_PDM_SOURCE_INIT(RimReservoirCellResultsCacheEntryInfo, "ResultCacheEntryInfo");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoirCellResultsCacheEntryInfo::RimReservoirCellResultsCacheEntryInfo()
{
    CAF_PDM_InitObject("Cache Entry", "", "", "");

    CAF_PDM_InitField(&m_resultType, "ResultType",  caf::AppEnum<RimDefines::ResultCatType>(RimDefines::REMOVED), "ResultType", "", "" ,"");
    CAF_PDM_InitField(&m_resultName, "ResultName",  QString(), "ResultName", "", "" ,"");
    CAF_PDM_InitFieldNoDefault(&m_timeStepDates, "TimeSteps", "TimeSteps", "", "" ,"");
    CAF_PDM_InitField(&m_filePosition, "FilePositionDataStart",  qint64(-1), "FilePositionDataStart", "", "" ,"");

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoirCellResultsCacheEntryInfo::~RimReservoirCellResultsCacheEntryInfo()
{

}
