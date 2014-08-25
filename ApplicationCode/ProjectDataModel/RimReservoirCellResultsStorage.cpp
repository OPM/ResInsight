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

#include "RimReservoirCellResultsStorage.h"

#include "RigCaseCellResultsData.h"
#include "RigActiveCellInfo.h"
#include "RigMainGrid.h"
#include "RigCell.h"
#include "RimTools.h"

#include "cafProgressInfo.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUuid>
#include "cvfGeometryTools.h"

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
    QString cacheDirPath = RimTools::getCacheRootDirectoryPathFromProject();
    cacheDirPath += "_cache";
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
/// This method is intended to be used for multicase cross statistical calculations, when 
/// we need process one timestep at a time, freeing memory as we go.
//--------------------------------------------------------------------------------------------------
size_t RimReservoirCellResultsStorage::findOrLoadScalarResultForTimeStep(RimDefines::ResultCatType type, const QString& resultName, size_t timeStepIndex)
{
    if (!m_cellResults) return cvf::UNDEFINED_SIZE_T;

    // Special handling for SOIL
    if (type == RimDefines::DYNAMIC_NATIVE && resultName.toUpper() == "SOIL")
    {
        size_t soilScalarResultIndex = m_cellResults->findScalarResultIndex(type, resultName);
        if (!isDataPresent(soilScalarResultIndex))
        {
            computeSOILForTimeStep(timeStepIndex);
        }

        return soilScalarResultIndex;
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

    size_t scalarResultIndex = m_cellResults->findScalarResultIndex(type, resultName);
    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T) return cvf::UNDEFINED_SIZE_T;

   
    if (isDataPresent(scalarResultIndex))
    {
        return scalarResultIndex;
    }

    if (resultName == "SOIL")
    {
        // Trigger loading of SWAT, SGAS to establish time step count if no data has been loaded from file at this point
        findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SWAT");
        findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SGAS");

        for (size_t timeStepIdx = 0; timeStepIdx < m_cellResults->maxTimeStepCount(); timeStepIdx++)
        {
            computeSOILForTimeStep(timeStepIdx);
        }

        return scalarResultIndex;
    }

    if (resultName == RimDefines::combinedRiTransResultName())
    {
        computeRiTransComponent(RimDefines::riTransXResultName());
        computeRiTransComponent(RimDefines::riTransYResultName());
        computeRiTransComponent(RimDefines::riTransZResultName());
    }
    
    if (   resultName == RimDefines::riTransXResultName()
        || resultName == RimDefines::riTransYResultName()
        || resultName == RimDefines::riTransZResultName())
    {
        computeRiTransComponent(resultName);
    }

    if (type == RimDefines::GENERATED)
    {
        return cvf::UNDEFINED_SIZE_T;
    }

    if (m_readerInterface.notNull())
    {
        // Add one more result to result container
        size_t timeStepCount = m_cellResults->infoForEachResultIndex()[scalarResultIndex].m_timeStepDates.size();

        bool resultLoadingSucess = true;

        if (type == RimDefines::DYNAMIC_NATIVE && timeStepCount > 0)
        {
            m_cellResults->cellScalarResults(scalarResultIndex).resize(timeStepCount);

            size_t i;
            for (i = 0; i < timeStepCount; i++)
            {
                std::vector<double>& values = m_cellResults->cellScalarResults(scalarResultIndex)[i];
                if (!m_readerInterface->dynamicResult(resultName, RifReaderInterface::MATRIX_RESULTS, i, &values))
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
            // Remove last scalar result because loading of result failed
            m_cellResults->cellScalarResults(scalarResultIndex).clear();
        }
    }

    return scalarResultIndex;
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

    if (scalarIndexSWAT != cvf::UNDEFINED_SIZE_T)
    {
        std::vector<double>& swatForTimeStep = m_cellResults->cellScalarResults(scalarIndexSWAT, timeStepIndex);
        if (swatForTimeStep.size() > 0)
        {
            soilResultValueCount = swatForTimeStep.size();
            soilTimeStepCount = m_cellResults->infoForEachResultIndex()[scalarIndexSWAT].m_timeStepDates.size();
        }
    }

    if (scalarIndexSGAS != cvf::UNDEFINED_SIZE_T)
    {
        std::vector<double>& sgasForTimeStep = m_cellResults->cellScalarResults(scalarIndexSGAS, timeStepIndex);
        if (sgasForTimeStep.size() > 0)
        {
            soilResultValueCount = qMax(soilResultValueCount, sgasForTimeStep.size());

            size_t sgasTimeStepCount = m_cellResults->infoForEachResultIndex()[scalarIndexSGAS].m_timeStepDates.size();
            soilTimeStepCount = qMax(soilTimeStepCount, sgasTimeStepCount);
        }
    }


    // Make sure memory is allocated for the new SOIL results

    size_t soilResultScalarIndex = m_cellResults->findScalarResultIndex(RimDefines::DYNAMIC_NATIVE, "SOIL");
    m_cellResults->cellScalarResults(soilResultScalarIndex).resize(soilTimeStepCount);
    m_cellResults->cellScalarResults(soilResultScalarIndex, timeStepIndex).resize(soilResultValueCount);


    std::vector<double>* swatForTimeStep = NULL;
    std::vector<double>* sgasForTimeStep = NULL;

    if (scalarIndexSWAT != cvf::UNDEFINED_SIZE_T)
    {
        swatForTimeStep = &(m_cellResults->cellScalarResults(scalarIndexSWAT, timeStepIndex));
        if (swatForTimeStep->size() == 0)
        {
            swatForTimeStep = NULL;
        }
    }

    if (scalarIndexSGAS != cvf::UNDEFINED_SIZE_T)
    {
        sgasForTimeStep = &(m_cellResults->cellScalarResults(scalarIndexSGAS, timeStepIndex));
        if (sgasForTimeStep->size() == 0)
        {
            sgasForTimeStep = NULL;
        }
    }

    std::vector<double>& soilForTimeStep = m_cellResults->cellScalarResults(soilResultScalarIndex, timeStepIndex);

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

#if 1

void calculateConnectionGeometry(const RigCell& c1, const RigCell& c2 , const std::vector<cvf::Vec3d>& nodes,  
                                 cvf::StructGridInterface::FaceType faceId,
                                 cvf::Vec3d* faceCenter, cvf::Vec3d* faceAreaVec)
{
    CVF_TIGHT_ASSERT(faceCenter && faceAreaVec);

    *faceCenter = cvf::Vec3d::ZERO;
    *faceAreaVec = cvf::Vec3d::ZERO;

    std::vector<size_t> polygon;
    std::vector<cvf::Vec3d> intersections;
    caf::SizeTArray4 face1;
    caf::SizeTArray4 face2;
    c1.faceIndices(faceId, &face1);
    c2.faceIndices(cvf::StructGridInterface::oppositeFace(faceId), &face2);

    bool foundOverlap = cvf::GeometryTools::calculateOverlapPolygonOfTwoQuads(
        &polygon,
        &intersections,
        (cvf::EdgeIntersectStorage<size_t>*)NULL,
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

        // Polygon center
        for (size_t pIdx = 0; pIdx < realPolygon.size(); ++pIdx)
        {
            *faceCenter += realPolygon[pIdx];
        }

        *faceCenter *= 1.0 / realPolygon.size();

        // Polygon area vector

        *faceAreaVec = cvf::GeometryTools::polygonAreaNormal3D(realPolygon);

    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double halfCellTransmissibility(double perm, double ntg, const cvf::Vec3d& centerToFace, const cvf::Vec3d& faceAreaVec)
{
    return perm*ntg*(faceAreaVec*centerToFace) / (centerToFace*centerToFace);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double newtran(double cdarchy, double mult, double halfCellTrans, double neighborHalfCellTrans)
{
    return cdarchy * mult / ((1 / halfCellTrans) + (1 / neighborHalfCellTrans));
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirCellResultsStorage::computeRiTransComponent(const QString& riTransComponentResultName)
{
    if (!m_cellResults) return;

    // Set up which component to compute

    cvf::StructGridInterface::FaceType faceId;
    QString permCompName;

    if (riTransComponentResultName == RimDefines::riTransXResultName())
    {
        permCompName = "PERMX";
        faceId = cvf::StructGridInterface::POS_I;
    }
    else if (riTransComponentResultName == RimDefines::riTransYResultName())
    {
        permCompName = "PERMY";
        faceId = cvf::StructGridInterface::POS_J;
    }
    else if (riTransComponentResultName == RimDefines::riTransZResultName())
    {
        permCompName = "PERMZ";
        faceId = cvf::StructGridInterface::POS_K;
    }
    else
    {
        CVF_ASSERT(false);
    }

    // Todo: Get the correct one from Unit set read by ERT
    double cdarchy = 0.008527; // (ECLIPSE 100) (METRIC)

    // Get the needed result indices we depend on

    size_t permResultIdx = findOrLoadScalarResult(RimDefines::STATIC_NATIVE, permCompName);
    size_t ntgResultIdx  = findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "NTG");
 
    // Get the result index of the output

    size_t riTransResultIdx = m_cellResults->findScalarResultIndex(RimDefines::STATIC_NATIVE, riTransComponentResultName);
    CVF_ASSERT(riTransResultIdx != cvf::UNDEFINED_SIZE_T);

    // Get the result count, to handle that one of them might be globally defined

    size_t permxResultValueCount = m_cellResults->cellScalarResults(permResultIdx)[0].size();
    size_t ntgResultValueCount = m_cellResults->cellScalarResults(ntgResultIdx)[0].size();

    size_t resultValueCount = CVF_MIN(permxResultValueCount, ntgResultValueCount);

    // Get all the actual result values

    std::vector<double> & permResults    = m_cellResults->cellScalarResults(permResultIdx)[0];
    std::vector<double> & ntgResults     = m_cellResults->cellScalarResults(ntgResultIdx)[0];
    std::vector<double> & riTransResults = m_cellResults->cellScalarResults(riTransResultIdx)[0];

    // Set up output container to correct number of results

    riTransResults.resize(resultValueCount);

    // Prepare how to index the result values:

    bool isPermUsingResIdx  = m_cellResults->isUsingGlobalActiveIndex(permResultIdx);
    bool isNtgUsingResIdx   = m_cellResults->isUsingGlobalActiveIndex(ntgResultIdx);
    bool isTransUsingResIdx = m_cellResults->isUsingGlobalActiveIndex(riTransResultIdx);
    
    // Set up result index function pointers

    ResultIndexFunction riTranIdxFunc   = isTransUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;
    ResultIndexFunction permIdxFunc     = isPermUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;
    ResultIndexFunction ntgIdxFunc      = isNtgUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;


    const RigActiveCellInfo* activeCellInfo = m_cellResults->activeCellInfo();
    const std::vector<cvf::Vec3d>& nodes = m_ownerMainGrid->nodes();
    bool isFaceNormalsOutwards = m_ownerMainGrid->faceNormalsIsOutwards();

    for (size_t nativeResvCellIndex = 0; nativeResvCellIndex < m_ownerMainGrid->cells().size(); nativeResvCellIndex++)
    {
        // Do nothing if we are only dealing with active cells, and this cell is not active:
        size_t tranResIdx = (*riTranIdxFunc)(activeCellInfo, nativeResvCellIndex);

        if (tranResIdx == cvf::UNDEFINED_SIZE_T) continue;

        const RigCell& nativeCell = m_ownerMainGrid->cells()[nativeResvCellIndex];
        RigGridBase* grid = nativeCell.hostGrid();

        size_t gridLocalNativeCellIndex = nativeCell.gridLocalCellIndex();

        size_t i, j, k, gridLocalNeighborCellIdx;

        grid->ijkFromCellIndex(gridLocalNativeCellIndex, &i, &j, &k);

        if (grid->cellIJKNeighbor(i, j, k, faceId, &gridLocalNeighborCellIdx))
        {
            size_t neighborResvCellIdx = grid->reservoirCellIndex(gridLocalNeighborCellIdx);
            const RigCell& neighborCell = m_ownerMainGrid->cells()[neighborResvCellIdx];

            // Do nothing if neighbor cell has no results
            size_t neighborCellPermResIdx = (*permIdxFunc)(activeCellInfo, neighborResvCellIdx);
            if (neighborCellPermResIdx == cvf::UNDEFINED_SIZE_T) continue;
            
            // Connection geometry

            const RigFault* fault = grid->mainGrid()->findFaultFromCellIndexAndCellFace(nativeResvCellIndex, faceId);
            bool isOnFault = fault;

 
            cvf::Vec3d faceAreaVec;
            cvf::Vec3d faceCenter;

            if (isOnFault)
            {
                calculateConnectionGeometry( nativeCell, neighborCell, nodes,faceId, 
                                             &faceCenter, &faceAreaVec);
            }
            else
            {
                faceCenter = nativeCell.faceCenter(faceId);
                faceAreaVec = nativeCell.faceNormalWithAreaLenght(faceId);
            }

            if (!isFaceNormalsOutwards) faceAreaVec = -faceAreaVec;

            double halfCellTrans = 0;
            double neighborHalfCellTrans = 0;

            // Native cell half cell transm
            {
                cvf::Vec3d centerToFace = faceCenter - nativeCell.center();

                size_t permResIdx = (*permIdxFunc)(activeCellInfo, nativeResvCellIndex);
                double perm = permResults[permResIdx];

                double ntg = 1.0;
                if (faceId != cvf::StructGridInterface::POS_K)
                {
                    size_t ntgResIdx = (*ntgIdxFunc)(activeCellInfo, nativeResvCellIndex);
                    ntg = ntgResults[ntgResIdx];
                }

                halfCellTrans = halfCellTransmissibility(perm, ntg, centerToFace, faceAreaVec);
            }

            // Neighbor cell half cell transm
            {
                cvf::Vec3d centerToFace = faceCenter - neighborCell.center();

                double perm = permResults[neighborCellPermResIdx];

                double ntg = 1.0;
                if (faceId != cvf::StructGridInterface::POS_K)
                {
                    size_t ntgResIdx = (*ntgIdxFunc)(activeCellInfo, neighborResvCellIdx);
                    ntg = ntgResults[ntgResIdx];
                }

                neighborHalfCellTrans = halfCellTransmissibility(perm, ntg, centerToFace, -faceAreaVec);
            }

            riTransResults[tranResIdx] = newtran(cdarchy, 1.0, halfCellTrans, neighborHalfCellTrans);;
        }

    }
   
}

#endif

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

//--------------------------------------------------------------------------------------------------
///  If we have any results on any time step, assume we have loaded results
//--------------------------------------------------------------------------------------------------
bool RimReservoirCellResultsStorage::isDataPresent(size_t scalarResultIndex) const
{
    if (scalarResultIndex >= m_cellResults->resultCount())
    {
        return false;
    }

    const std::vector< std::vector<double> > data = m_cellResults->cellScalarResults(scalarResultIndex);

    for (size_t tsIdx = 0; tsIdx < data.size(); ++tsIdx)
    {
        if (data[tsIdx].size())
        {
            return true;
        }
    }

    return false;
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