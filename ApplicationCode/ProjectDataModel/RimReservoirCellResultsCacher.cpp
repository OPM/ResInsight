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

CAF_PDM_SOURCE_INIT(RimReservoirCellResultsCacher, "ReservoirCellResultCacher");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoirCellResultsCacher::RimReservoirCellResultsCacher()
    : m_cellResults(NULL)
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
            RimReservoirCellResultsCacheEntryInfo*  cacheEntry = new RimReservoirCellResultsCacheEntryInfo;
            m_resultCacheMetaData.push_back(cacheEntry);

            cacheEntry->m_resultType = resInfo[rIdx].m_resultType;
            cacheEntry->m_resultName = resInfo[rIdx].m_resultName;
            cacheEntry->m_timeStepDates = resInfo[rIdx].m_timeStepDates;

            cacheEntry->m_filePosition = cacheFile.pos();

            for (int tsIdx = 0; tsIdx < resInfo[rIdx].m_timeStepDates.size() ; ++tsIdx)
            {
                std::vector<double>& data = m_cellResults->cellScalarResults(resInfo[rIdx].m_gridScalarResultIndex, tsIdx);
                if (data.size())
                {
                    cacheEntry->m_timeStepHasData.v().push_back(1);

                    for (size_t cIdx = 0; cIdx < data.size(); ++cIdx)
                    {
                        stream << data[cIdx];
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
