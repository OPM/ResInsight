/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RimReservoirCellResultsStorage.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigEclipseResultInfo.h"

#include "RimEclipseCase.h"
#include "RimTools.h"
#include "RimProject.h"
#include "RimCompletionCellIntersectionCalc.h"

#include "cafProgressInfo.h"
#include "cafUtils.h"

#include "cvfGeometryTools.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUuid>
#include "RifReaderEclipseOutput.h"

CAF_PDM_SOURCE_INIT(RimReservoirCellResultsStorage, "ReservoirCellResultStorage");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoirCellResultsStorage::RimReservoirCellResultsStorage()
    : m_cellResults(NULL)
{
    CAF_PDM_InitObject("Cacher", "", "", "");

    CAF_PDM_InitField(&m_resultCacheFileName, "ResultCacheFileName",  QString(), "UiDummyname", "", "" ,"");
    m_resultCacheFileName.uiCapability()->setUiHidden(true);
    CAF_PDM_InitFieldNoDefault(&m_resultCacheMetaData, "ResultCacheEntries", "UiDummyname", "", "", "");
    m_resultCacheMetaData.uiCapability()->setUiHidden(true);

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

    const std::vector<RigEclipseResultInfo>&  resInfo = m_cellResults->infoForEachResultIndex();

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
                cacheEntry->m_timeStepDates = resInfo[rIdx].dates();
                cacheEntry->m_daysSinceSimulationStart = resInfo[rIdx].daysSinceSimulationStarts();

                // Take note of the file position for fast lookup later
                cacheEntry->m_filePosition = cacheFile.pos();

                // Write all the scalar values for each time step to the stream, 
                // starting with the number of values 
                for (size_t tsIdx = 0; tsIdx < resInfo[rIdx].dates().size() ; ++tsIdx)
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
    QString cacheDirPath = RimTools::getCacheRootDirectoryPathFromProject();
    cacheDirPath += "_cache";
    return cacheDirPath;
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

    // Warn if we thought we were to find some data on the storage file

    if (!caf::Utils::fileExists(newValidCacheFileName) && m_resultCacheMetaData.size())
    {
        qWarning() << "Reading stored results: Missing the storage file : " + newValidCacheFileName; 
        return;
    }

    QFile storageFile(newValidCacheFileName);
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
        size_t resultIndex = m_cellResults->findOrCreateScalarResultIndex(resInfo->m_resultType(), resInfo->m_resultName(), true);

        std::vector<int> reportNumbers; // Hack: Using no report step numbers. Not really used except for Flow Diagnostics...
        reportNumbers.resize(resInfo->m_timeStepDates().size());
        std::vector<RigEclipseTimeStepInfo> timeStepInfos = RigEclipseTimeStepInfo::createTimeStepInfos(resInfo->m_timeStepDates(), reportNumbers, resInfo->m_daysSinceSimulationStart());

        m_cellResults->setTimeStepInfos(resultIndex, timeStepInfos);

        progress.setProgressDescription(resInfo->m_resultName);

        for (size_t tsIdx = 0; tsIdx < resInfo->m_timeStepDates().size(); ++tsIdx)
        {
            std::vector<double>* data = NULL;

            data = &(m_cellResults->cellScalarResults(resultIndex, tsIdx));

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

    CAF_PDM_InitField(&m_resultType, "ResultType",  caf::AppEnum<RiaDefines::ResultCatType>(RiaDefines::REMOVED), "ResultType", "", "" ,"");
    CAF_PDM_InitField(&m_resultName, "ResultName",  QString(), "ResultName", "", "" ,"");
    CAF_PDM_InitFieldNoDefault(&m_timeStepDates, "TimeSteps", "TimeSteps", "", "" ,"");
    CAF_PDM_InitFieldNoDefault(&m_daysSinceSimulationStart, "DaysSinceSimulationStart", "DaysSinceSimulationStart", "", "", "");
    CAF_PDM_InitField(&m_filePosition, "FilePositionDataStart",  qint64(-1), "FilePositionDataStart", "", "" ,"");

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoirCellResultsStorageEntryInfo::~RimReservoirCellResultsStorageEntryInfo()
{

}
