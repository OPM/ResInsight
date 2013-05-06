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

//#include "RiaStdInclude.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QUuid>

#include "RimReservoirCellResultsCacher.h"
#include "RigCaseCellResultsData.h"
#include "RiaApplication.h"
#include "RigMainGrid.h"
#include "RigCell.h"
#include "cafProgressInfo.h"
#include "RimProject.h"
#include "RimCase.h"
#include "RimCaseCollection.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimScriptCollection.h"
#include "RimReservoirView.h"

#include "cafPdmFieldCvfMat4d.h"
#include "cafPdmFieldCvfColor.h"
#include "RimResultSlot.h"
#include "RimCellEdgeResultSlot.h"
#include "RimCellRangeFilterCollection.h"
#include "RimCellPropertyFilterCollection.h"
#include "RimWellCollection.h"
#include "Rim3dOverlayInfoConfig.h"

CAF_PDM_SOURCE_INIT(RimReservoirCellResultsStorage, "ReservoirCellResultStorage");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoirCellResultsStorage::RimReservoirCellResultsStorage()
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
RimReservoirCellResultsStorage::~RimReservoirCellResultsStorage()
{
    m_resultCacheMetaData.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// This override populates the metainfo regarding the cell results data in the RigCaseCellResultsData
/// object. This metainfo will then be written to the project file when saving, and thus read on project file open.
/// This method then writes the actual double arrays to the data file in a simple format:
/// MagicNumber<uint32>, Version<uint32>, ResultVariables< Array < TimeStep< CellDataArraySize<uint64>, CellData< Array<double > > > >
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirCellResultsStorage::setupBeforeSave()
{
    m_resultCacheMetaData.deleteAllChildObjects();
    QString newValidCacheFileName = getValidCacheFileName();

    // Delete the storage file

    QFileInfo storageFileInfo(newValidCacheFileName);
    if (storageFileInfo.exists())
    {
        QDir storageDir = storageFileInfo.dir();
        storageDir.remove(storageFileInfo.fileName()); 
    }

    if (!m_cellResults) return;

    const std::vector<RigCaseCellResultsData::ResultInfo>&  resInfo = m_cellResults->infoForEachResultIndex();

    bool hasResultsToStore = false;
    for (size_t rIdx = 0; rIdx < resInfo.size(); ++rIdx) 
    {
        if (resInfo[rIdx].m_needsToBeStored) 
        {
            hasResultsToStore = true; 
            break;
        }
    }

    if(resInfo.size() && hasResultsToStore)
    {
        QDir::root().mkpath(getCacheDirectoryPath());

        QFile cacheFile(newValidCacheFileName);

        if (!cacheFile.open(QIODevice::WriteOnly)) 
        {
            qWarning() << "Saving project: Can't open the cache file : " + newValidCacheFileName; 
            return;
        }

        m_resultCacheFileName = newValidCacheFileName;

        QDataStream stream(&cacheFile);
        stream.setVersion(QDataStream::Qt_4_6);
        stream << (quint32)0xCEECAC4E; // magic number
        stream << (quint32)1; // Version number. Increment if needing to extend the format in ways that can not be handled generically by the reader

        caf::ProgressInfo progInfo(resInfo.size(), "Saving generated and imported properties");

        for (size_t rIdx = 0; rIdx < resInfo.size(); ++rIdx)
        {
            // If there is no data, we do not store anything for the current result variable
            // (Even not the metadata, of cause)
            size_t timestepCount = m_cellResults->cellScalarResults(resInfo[rIdx].m_gridScalarResultIndex).size();
            
            if (timestepCount && resInfo[rIdx].m_needsToBeStored)
            {
                progInfo.setProgressDescription(resInfo[rIdx].m_resultName);

                // Create and setup the cache information for this result
                RimReservoirCellResultsStorageEntryInfo*  cacheEntry = new RimReservoirCellResultsStorageEntryInfo;
                m_resultCacheMetaData.push_back(cacheEntry);

                cacheEntry->m_resultType = resInfo[rIdx].m_resultType;
                cacheEntry->m_resultName = resInfo[rIdx].m_resultName;
                cacheEntry->m_timeStepDates = resInfo[rIdx].m_timeStepDates;

                // Take note of the file position for fast lookup later
                cacheEntry->m_filePosition = cacheFile.pos();

                // Write all the scalar values for each time step to the stream, 
                // starting with the number of values 
                for (size_t tsIdx = 0; tsIdx < resInfo[rIdx].m_timeStepDates.size() ; ++tsIdx)
                {
                    const std::vector<double>* data = NULL;
                    if (tsIdx < timestepCount)
                    {
                        data = &(m_cellResults->cellScalarResults(resInfo[rIdx].m_gridScalarResultIndex, tsIdx));
                    }

                    if (data && data->size())
                    {

                        stream << (quint64)(data->size());
                        for (size_t cIdx = 0; cIdx < data->size(); ++cIdx)
                        {
                            stream << (*data)[cIdx];
                        }
                    }
                    else
                    {
                        stream << (quint64)0;
                    }
                }
            }

            progInfo.incrementProgress();
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimReservoirCellResultsStorage::getValidCacheFileName()
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
QString RimReservoirCellResultsStorage::getCacheDirectoryPath()
{
    QString cacheDirPath;
    QString projectFileName = RiaApplication::instance()->project()->fileName();
    QFileInfo fileInfo(projectFileName);
    cacheDirPath = fileInfo.canonicalPath();
    cacheDirPath += "/" + fileInfo.completeBaseName() + "_cache";
    return cacheDirPath;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirCellResultsStorage::setReaderInterface(RifReaderInterface* readerInterface)
{
    m_readerInterface = readerInterface;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderInterface* RimReservoirCellResultsStorage::readerInterface()
{
    return m_readerInterface.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimReservoirCellResultsStorage::findOrLoadScalarResultForTimeStep(RimDefines::ResultCatType type, const QString& resultName, size_t timeStepIndex)
{
    if (!m_cellResults) return cvf::UNDEFINED_SIZE_T;

    // Special handling for SOIL
    if (type == RimDefines::DYNAMIC_NATIVE && resultName.toUpper() == "SOIL")
    {
        size_t soilScalarResultIndex = m_cellResults->findScalarResultIndex(type, resultName);

        // If SOIL is not found, try to compute and return computed scalar index
        // Will return cvf::UNDEFINED_SIZE_T if no SGAS/SWAT is found
        if (soilScalarResultIndex == cvf::UNDEFINED_SIZE_T)
        {
            computeSOILForTimeStep(timeStepIndex);
            
            soilScalarResultIndex = m_cellResults->findScalarResultIndex(type, resultName);
            return soilScalarResultIndex;
        }

        // If we have found SOIL and SOIL must be calculated, calculate and return
        if (soilScalarResultIndex != cvf::UNDEFINED_SIZE_T && m_cellResults->mustBeCalculated(soilScalarResultIndex))
        {
            computeSOILForTimeStep(timeStepIndex);

            return soilScalarResultIndex;
        }
    }

    size_t scalarResultIndex = m_cellResults->findScalarResultIndex(type, resultName);
    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T) return cvf::UNDEFINED_SIZE_T;

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
size_t RimReservoirCellResultsStorage::findOrLoadScalarResult(RimDefines::ResultCatType type, const QString& resultName)
{
    if (!m_cellResults) return cvf::UNDEFINED_SIZE_T;

    size_t resultGridIndex = m_cellResults->findScalarResultIndex(type, resultName);
    if (resultGridIndex == cvf::UNDEFINED_SIZE_T) return cvf::UNDEFINED_SIZE_T;

    // If we have any results on any timestep, assume we have loaded results already

    for (size_t tsIdx = 0; tsIdx < m_cellResults->timeStepCount(resultGridIndex); ++tsIdx)
    {
        if (m_cellResults->cellScalarResults(resultGridIndex, tsIdx).size())
        {
            return resultGridIndex;
        }
    }

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
void RimReservoirCellResultsStorage::loadOrComputeSOIL()
{
    size_t scalarIndexSOIL = findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SOIL");
    if (scalarIndexSOIL != cvf::UNDEFINED_SIZE_T)
    {
        return;
    }

    for (size_t timeStepIdx = 0; timeStepIdx < m_cellResults->maxTimeStepCount(); timeStepIdx++)
    {
        computeSOILForTimeStep(timeStepIdx);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirCellResultsStorage::computeSOILForTimeStep(size_t timeStepIndex)
{
    size_t scalarIndexSWAT = findOrLoadScalarResultForTimeStep(RimDefines::DYNAMIC_NATIVE, "SWAT", timeStepIndex);
    size_t scalarIndexSGAS = findOrLoadScalarResultForTimeStep(RimDefines::DYNAMIC_NATIVE, "SGAS", timeStepIndex);

    // Early exit if none of SWAT or SGAS is present
    if (scalarIndexSWAT == cvf::UNDEFINED_SIZE_T && scalarIndexSGAS == cvf::UNDEFINED_SIZE_T)
    {
        return;
    }

    CVF_ASSERT(m_cellResults);

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
        soilResultGridIndex = m_cellResults->addEmptyScalarResult(RimDefines::DYNAMIC_NATIVE, "SOIL", false);
        CVF_ASSERT(soilResultGridIndex != cvf::UNDEFINED_SIZE_T);

        // Set this result to be calculated
        m_cellResults->setMustBeCalculated(soilResultGridIndex);

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
void RimReservoirCellResultsStorage::computeDepthRelatedResults()
{
    if (!m_cellResults) return;

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
         depthResultGridIndex = m_cellResults->addStaticScalarResult(RimDefines::STATIC_NATIVE, "DEPTH", false, resultValueCount);
        computeDepth = true;
    }

    if (dxResultGridIndex == cvf::UNDEFINED_SIZE_T)
    {
        dxResultGridIndex = m_cellResults->addStaticScalarResult(RimDefines::STATIC_NATIVE, "DX", false, resultValueCount);
        computeDx = true;
    }

    if (dyResultGridIndex == cvf::UNDEFINED_SIZE_T)
    {
        dyResultGridIndex = m_cellResults->addStaticScalarResult(RimDefines::STATIC_NATIVE, "DY", false, resultValueCount);
        computeDy = true;
    }

    if (dzResultGridIndex == cvf::UNDEFINED_SIZE_T)
    {
        dzResultGridIndex = m_cellResults->addStaticScalarResult(RimDefines::STATIC_NATIVE, "DZ", false, resultValueCount);
        computeDz = true;
    }

    if (topsResultGridIndex == cvf::UNDEFINED_SIZE_T)
    {
        topsResultGridIndex = m_cellResults->addStaticScalarResult(RimDefines::STATIC_NATIVE, "TOPS", false, resultValueCount);
        computeTops = true;
    }

    if (bottomResultGridIndex == cvf::UNDEFINED_SIZE_T)
    {
        bottomResultGridIndex = m_cellResults->addStaticScalarResult(RimDefines::STATIC_NATIVE, "BOTTOM", false, resultValueCount);
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
size_t RimReservoirCellResultsStorage::findOrLoadScalarResult(const QString& resultName)
{
    if (!m_cellResults) return cvf::UNDEFINED_SIZE_T;

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
void RimReservoirCellResultsStorage::setCellResults(RigCaseCellResultsData* cellResults)
{
    m_cellResults = cellResults;

    if (m_cellResults == NULL) 
        return;

    // Now that we have got the results container, we can finally 
    // Read data from the internal storage and populate it

    if (m_resultCacheFileName().isEmpty()) 
        return;

    // Get the name of the cache name relative to the current project file position
    QString newValidCacheFileName = getValidCacheFileName();

    QFile storageFile(newValidCacheFileName);

    // Warn if we thought we were to find some data on the storage file

    if (!storageFile.exists() && m_resultCacheMetaData.size())
    {
        qWarning() << "Reading stored results: Missing the storage file : " + newValidCacheFileName; 
        return;
    }

    if (!storageFile.open(QIODevice::ReadOnly)) 
    {
        qWarning() << "Reading stored results: Can't open the file : " + newValidCacheFileName; 
        return;
    }

    QDataStream stream(&storageFile);
    stream.setVersion(QDataStream::Qt_4_6);
    quint32 magicNumber = 0;
    quint32 versionNumber = 0;
    stream >> magicNumber;

    if (magicNumber != 0xCEECAC4E)
    {
        qWarning() << "Reading stored results: The storage file has wrong type "; 
        return;
    }

    stream >> versionNumber;
    if (versionNumber > 1 )
    {
        qWarning() << "Reading stored results: The storage file has been written by a newer version of ResInsight"; 
        return;
    }

    caf::ProgressInfo progress(m_resultCacheMetaData.size(), "Reading internally stored results");
    // Fill the object with data from the storage

    for (size_t rIdx = 0; rIdx < m_resultCacheMetaData.size(); ++rIdx)
    {
        RimReservoirCellResultsStorageEntryInfo* resInfo = m_resultCacheMetaData[rIdx];
        size_t resultIndex = m_cellResults->addEmptyScalarResult(resInfo->m_resultType(), resInfo->m_resultName(), true);

        m_cellResults->setTimeStepDates(resultIndex, resInfo->m_timeStepDates());

        progress.setProgressDescription(resInfo->m_resultName);

        for (size_t tsIdx = 0; tsIdx < resInfo->m_timeStepDates().size(); ++tsIdx)
        {
            std::vector<double>* data = NULL;
           
            data = &(m_cellResults->cellScalarResults(rIdx, tsIdx));

            quint64 cellCount = 0;
            stream >> cellCount;
            data->resize(cellCount, HUGE_VAL);

            for (size_t cIdx = 0; cIdx < cellCount; ++cIdx)
            {
                stream >> (*data)[cIdx];
            }
        }

        progress.incrementProgress();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirCellResultsStorage::setMainGrid(RigMainGrid* mainGrid)
{
    m_ownerMainGrid = mainGrid;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimReservoirCellResultsStorage::storedResultsCount()
{
    return m_resultCacheMetaData.size();
}


CAF_PDM_SOURCE_INIT(RimReservoirCellResultsStorageEntryInfo, "ResultStorageEntryInfo");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoirCellResultsStorageEntryInfo::RimReservoirCellResultsStorageEntryInfo()
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
RimReservoirCellResultsStorageEntryInfo::~RimReservoirCellResultsStorageEntryInfo()
{

}
