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

#include "RifReaderEclipseOutput.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "RifEclipseInputFileTools.h"
#include "RifEclipseOutputFileTools.h"
#include "RifHdf5ReaderInterface.h"
#include "RifReaderSettings.h"

#ifdef USE_HDF5
#include "RifHdf5Reader.h"
#endif

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigSingleWellResultsData.h"
#include "RigEclipseResultInfo.h"

#include "cafProgressInfo.h"

#include "cvfTrace.h"

#include "ert/ecl/ecl_kw_magic.h"
#include "ert/ecl/ecl_nnc_export.h"
#include "ert/ecl/ecl_nnc_geometry.h"
#include "ert/ecl/ecl_nnc_data.h"

#include <cmath> // Needed for HUGE_VAL on Linux
#include <iostream>
#include <map>


//--------------------------------------------------------------------------------------------------
///     ECLIPSE cell numbering layout:
///        Lower layer:   Upper layer
///        Low Depth      High Depth
///        Low K          High K
///        Shallow        Deep
///       2---3           6---7
///       |   |           |   |
///       0---1           4---5
///
///
///
//--------------------------------------------------------------------------------------------------

// The indexing conventions for vertices in ECLIPSE
//
//      2-------------3              
//     /|            /|                  
//    / |           / |               /j   
//   /  |          /  |              /     
//  0-------------1   |             *---i  
//  |   |         |   |             | 
//  |   6---------|---7             |
//  |  /          |  /              |k
//  | /           | /
//  |/            |/
//  4-------------5
//  vertex indices
//
// The indexing conventions for vertices in ResInsight
//
//      7-------------6             |k     
//     /|            /|             | /j   
//    / |           / |             |/     
//   /  |          /  |             *---i  
//  4-------------5   |
//  |   |         |   |
//  |   3---------|---2
//  |  /          |  /
//  | /           | /
//  |/            |/
//  0-------------1
//  vertex indices
//

static const size_t cellMappingECLRi[8] = { 0, 1, 3, 2, 4, 5, 7, 6 };


//**************************************************************************************************
// Static functions
//**************************************************************************************************

bool transferGridCellData(RigMainGrid* mainGrid, RigActiveCellInfo* activeCellInfo, RigActiveCellInfo* fractureActiveCellInfo, RigGridBase* localGrid, const ecl_grid_type* localEclGrid, size_t matrixActiveStartIndex, size_t fractureActiveStartIndex)
{
    CVF_ASSERT(activeCellInfo && fractureActiveCellInfo);

    int cellCount = ecl_grid_get_global_size(localEclGrid);
    size_t cellStartIndex = mainGrid->globalCellArray().size();
    size_t nodeStartIndex = mainGrid->nodes().size();

    RigCell defaultCell;
    defaultCell.setHostGrid(localGrid);
    mainGrid->globalCellArray().resize(cellStartIndex + cellCount, defaultCell);

    mainGrid->nodes().resize(nodeStartIndex + cellCount*8, cvf::Vec3d(0,0,0));

    int progTicks = 100;
    double cellsPrProgressTick = cellCount/(float)progTicks;
    caf::ProgressInfo progInfo(progTicks, "");
    size_t computedCellCount = 0;
    // Loop over cells and fill them with data

#pragma omp parallel for
    for (int gridLocalCellIndex = 0; gridLocalCellIndex < cellCount; ++gridLocalCellIndex)
    {
        RigCell& cell = mainGrid->globalCellArray()[cellStartIndex + gridLocalCellIndex];

        cell.setGridLocalCellIndex(gridLocalCellIndex);

        // Active cell index

        int matrixActiveIndex = ecl_grid_get_active_index1(localEclGrid, gridLocalCellIndex);
        if (matrixActiveIndex != -1)
        {
            activeCellInfo->setCellResultIndex(cellStartIndex + gridLocalCellIndex, matrixActiveStartIndex + matrixActiveIndex);
        }

        int fractureActiveIndex = ecl_grid_get_active_fracture_index1(localEclGrid, gridLocalCellIndex);
        if (fractureActiveIndex != -1)
        {
            fractureActiveCellInfo->setCellResultIndex(cellStartIndex + gridLocalCellIndex, fractureActiveStartIndex + fractureActiveIndex);
        }

        // Parent cell index

        int parentCellIndex = ecl_grid_get_parent_cell1(localEclGrid, gridLocalCellIndex);
        if (parentCellIndex == -1)
        {
            cell.setParentCellIndex(cvf::UNDEFINED_SIZE_T);
        }
        else
        {
            cell.setParentCellIndex(parentCellIndex);
        }
    
        // Corner coordinates
        int cIdx;
        for (cIdx = 0; cIdx < 8; ++cIdx)
        {
            double * point = mainGrid->nodes()[nodeStartIndex + gridLocalCellIndex * 8 + cellMappingECLRi[cIdx]].ptr();
            ecl_grid_get_cell_corner_xyz1(localEclGrid, gridLocalCellIndex, cIdx, &(point[0]), &(point[1]), &(point[2]));
            point[2] = -point[2]; // Flipping Z making depth become negative z values
            cell.cornerIndices()[cIdx] = nodeStartIndex + gridLocalCellIndex*8 + cIdx;
        }

        // Sub grid in cell
        const ecl_grid_type* subGrid = ecl_grid_get_cell_lgr1(localEclGrid, gridLocalCellIndex);
        if (subGrid != NULL)
        {
            int subGridId = ecl_grid_get_lgr_nr(subGrid);
            CVF_ASSERT(subGridId > 0);
            cell.setSubGrid(static_cast<RigLocalGrid*>(mainGrid->gridById(subGridId)));
        }

        // Mark inactive long pyramid looking cells as invalid
        // Forslag
        //if (!invalid && (cell.isInCoarseCell() || (!cell.isActiveInMatrixModel() && !cell.isActiveInFractureModel()) ) )
        cell.setInvalid(cell.isLongPyramidCell());

#pragma omp atomic
        computedCellCount++;

        progInfo.setProgress((int)(computedCellCount/cellsPrProgressTick));
    }
    return true;
}

//==================================================================================================
//
// Class RigReaderInterfaceECL
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
RifReaderEclipseOutput::RifReaderEclipseOutput()
{
    m_fileName.clear();
    m_filesWithSameBaseName.clear();

    m_eclipseCase = NULL;

    m_ecl_init_file = NULL;
    m_dynamicResultsAccess = NULL;
}

//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
RifReaderEclipseOutput::~RifReaderEclipseOutput()
{
    close();

    if (m_ecl_init_file)
    {
        ecl_file_close(m_ecl_init_file);
    }
    m_ecl_init_file = NULL;

    if (m_dynamicResultsAccess.notNull())
    {
        m_dynamicResultsAccess->close();
    }

}


//--------------------------------------------------------------------------------------------------
/// Close interface (for now, no files are kept open after calling methods, so just clear members)
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::close()
{
}

//--------------------------------------------------------------------------------------------------
/// Read geometry from file given by name into given reservoir object
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::transferGeometry(const ecl_grid_type* mainEclGrid, RigEclipseCaseData* eclipseCase)
{
    CVF_ASSERT(eclipseCase);

    if (!mainEclGrid)
    {
        // Some error
        return false;
    }

    RigActiveCellInfo* activeCellInfo = eclipseCase->activeCellInfo(RiaDefines::MATRIX_MODEL);
    RigActiveCellInfo* fractureActiveCellInfo = eclipseCase->activeCellInfo(RiaDefines::FRACTURE_MODEL);

    CVF_ASSERT(activeCellInfo && fractureActiveCellInfo);

    RigMainGrid* mainGrid = eclipseCase->mainGrid();
    CVF_ASSERT(mainGrid);
    {
        cvf::Vec3st  gridPointDim(0,0,0);
        gridPointDim.x() = ecl_grid_get_nx(mainEclGrid) + 1;
        gridPointDim.y() = ecl_grid_get_ny(mainEclGrid) + 1;
        gridPointDim.z() = ecl_grid_get_nz(mainEclGrid) + 1;
        mainGrid->setGridPointDimensions(gridPointDim);
    }

    // std::string mainGridName = ecl_grid_get_name(mainEclGrid);
    // ERT returns file path to grid file as name for main grid
    mainGrid->setGridName("Main grid");

    // Get and set grid and lgr metadata

    size_t totalCellCount = static_cast<size_t>(ecl_grid_get_global_size(mainEclGrid));

    int numLGRs = ecl_grid_get_num_lgr(mainEclGrid);
    int lgrIdx;
    for (lgrIdx = 0; lgrIdx < numLGRs; ++lgrIdx)
    {
        ecl_grid_type* localEclGrid = ecl_grid_iget_lgr(mainEclGrid, lgrIdx);

        std::string lgrName = ecl_grid_get_name(localEclGrid);
        int lgrId = ecl_grid_get_lgr_nr(localEclGrid);

        cvf::Vec3st  gridPointDim(0,0,0);
        gridPointDim.x() = ecl_grid_get_nx(localEclGrid) + 1;
        gridPointDim.y() = ecl_grid_get_ny(localEclGrid) + 1;
        gridPointDim.z() = ecl_grid_get_nz(localEclGrid) + 1;

        RigLocalGrid* localGrid = new RigLocalGrid(mainGrid);
        localGrid->setGridId(lgrId);
        mainGrid->addLocalGrid(localGrid);

        localGrid->setIndexToStartOfCells(totalCellCount);
        localGrid->setGridName(lgrName);
        localGrid->setGridPointDimensions(gridPointDim);

        totalCellCount += ecl_grid_get_global_size(localEclGrid);
    }
    
    activeCellInfo->setReservoirCellCount(totalCellCount);
    fractureActiveCellInfo->setReservoirCellCount(totalCellCount);

    // Reserve room for the cells and nodes and fill them with data

    mainGrid->globalCellArray().reserve(totalCellCount);
    mainGrid->nodes().reserve(8*totalCellCount);

    caf::ProgressInfo progInfo(3 + numLGRs, "");
    progInfo.setProgressDescription("Main Grid");
    progInfo.setNextProgressIncrement(3);

    transferGridCellData(mainGrid, activeCellInfo, fractureActiveCellInfo, mainGrid, mainEclGrid, 0, 0);

    progInfo.setProgress(3);

    size_t globalMatrixActiveSize = ecl_grid_get_nactive(mainEclGrid);
    size_t globalFractureActiveSize = ecl_grid_get_nactive_fracture(mainEclGrid);

    activeCellInfo->setGridCount(1 + numLGRs);
    fractureActiveCellInfo->setGridCount(1 + numLGRs);

    activeCellInfo->setGridActiveCellCounts(0, globalMatrixActiveSize);
    fractureActiveCellInfo->setGridActiveCellCounts(0, globalFractureActiveSize);

    transferCoarseningInfo(mainEclGrid, mainGrid);


    for (lgrIdx = 0; lgrIdx < numLGRs; ++lgrIdx)
    {
        progInfo.setProgressDescription("LGR number " + QString::number(lgrIdx+1));

        ecl_grid_type* localEclGrid = ecl_grid_iget_lgr(mainEclGrid, lgrIdx);
        RigLocalGrid* localGrid = static_cast<RigLocalGrid*>(mainGrid->gridByIndex(lgrIdx+1));

        transferGridCellData(mainGrid, activeCellInfo, fractureActiveCellInfo, localGrid, localEclGrid, globalMatrixActiveSize, globalFractureActiveSize);

        int matrixActiveCellCount = ecl_grid_get_nactive(localEclGrid);
        globalMatrixActiveSize += matrixActiveCellCount;

        int fractureActiveCellCount = ecl_grid_get_nactive_fracture(localEclGrid);
        globalFractureActiveSize += fractureActiveCellCount;

        activeCellInfo->setGridActiveCellCounts(lgrIdx + 1, matrixActiveCellCount);
        fractureActiveCellInfo->setGridActiveCellCounts(lgrIdx + 1, fractureActiveCellCount);

        transferCoarseningInfo(localEclGrid, localGrid);

        progInfo.setProgress(3 + lgrIdx);
    }

    activeCellInfo->computeDerivedData();
    fractureActiveCellInfo->computeDerivedData();

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Open file and read geometry into given reservoir object
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::open(const QString& fileName, RigEclipseCaseData* eclipseCase)
{
    CVF_ASSERT(eclipseCase);
    caf::ProgressInfo progInfo(100, "");

    progInfo.setProgressDescription("Reading Grid");

    // Make sure everything's closed
    close();

    // Get set of files
    QStringList fileSet;
    if (!RifEclipseOutputFileTools::findSiblingFilesWithSameBaseName(fileName, &fileSet)) return false;

    m_fileName = fileName;
    
    progInfo.incrementProgress();

    progInfo.setNextProgressIncrement(20);
    // Keep the set of files of interest
    m_filesWithSameBaseName = fileSet;

    // Read geometry
    // Todo: Needs to check existence of file before calling ert, else it will abort
    ecl_grid_type * mainEclGrid = ecl_grid_alloc( fileName.toAscii().data() );

    progInfo.incrementProgress();

    progInfo.setNextProgressIncrement(10);
    progInfo.setProgressDescription("Transferring grid geometry");

    if (!transferGeometry(mainEclGrid, eclipseCase)) return false;

    progInfo.incrementProgress();
    progInfo.setProgressDescription("Reading faults");
    progInfo.setNextProgressIncrement(10);

    if (isFaultImportEnabled())
    {
        cvf::Collection<RigFault> faults;

        importFaults(fileSet, &faults);

        RigMainGrid* mainGrid = eclipseCase->mainGrid();
        mainGrid->setFaults(faults);
    }

    progInfo.incrementProgress();

    m_eclipseCase = eclipseCase;

    // Build results meta data
    progInfo.setProgressDescription("Reading Result index");
    progInfo.setNextProgressIncrement(25);
    buildMetaData();
    progInfo.incrementProgress();

    if (isNNCsEnabled())
    {
        progInfo.setProgressDescription("Reading NNC data");
        progInfo.setNextProgressIncrement(5);
        transferStaticNNCData(mainEclGrid, m_ecl_init_file, eclipseCase->mainGrid());
        progInfo.incrementProgress();

        transferDynamicNNCData(mainEclGrid, eclipseCase->mainGrid());
        progInfo.incrementProgress();

        progInfo.setProgressDescription("Processing NNC data");
        progInfo.setNextProgressIncrement(20);
        eclipseCase->mainGrid()->nncData()->processConnections( *(eclipseCase->mainGrid()));
        progInfo.incrementProgress();
    }
    else
    {
        progInfo.setNextProgressIncrement(25);
        progInfo.incrementProgress();
    }

    progInfo.setNextProgressIncrement(8);
    if (!RiaApplication::instance()->preferences()->readerSettings()->skipWellData())
    {
        progInfo.setProgressDescription("Reading Well information");
        readWellCells(mainEclGrid, isImportOfCompleteMswDataEnabled());
    }
    else
    {
        RiaLogging::info("Skipping import of simulation well data");
    }
    progInfo.incrementProgress();

    progInfo.setProgressDescription("Releasing reader memory");
    ecl_grid_free( mainEclGrid );
    progInfo.incrementProgress();

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::setHdf5FileName(const QString& fileName)
{
    CVF_ASSERT(m_eclipseCase);

    RigCaseCellResultsData* matrixModelResults = m_eclipseCase->results(RiaDefines::MATRIX_MODEL);
    CVF_ASSERT(matrixModelResults);

    if (fileName.isEmpty())
    {
        RiaLogging::info("HDF: Removing all existing Sour Sim data ...");
        matrixModelResults->eraseAllSourSimData();

        return;
    }

    RiaLogging::info(QString("HDF: Start import of data from : ").arg(fileName));

    RiaLogging::info("HDF: Removing all existing Sour Sim data ...");
    matrixModelResults->eraseAllSourSimData();

    std::vector<RigEclipseTimeStepInfo> timeStepInfos = createFilteredTimeStepInfos();

    std::unique_ptr<RifHdf5ReaderInterface> hdf5ReaderInterface;
#ifdef USE_HDF5
    hdf5ReaderInterface = std::unique_ptr<RifHdf5ReaderInterface>(new RifHdf5Reader(fileName));
#endif // USE_HDF5

    if (!hdf5ReaderInterface)
    {
        RiaLogging::error("HDF: Failed to import Sour Sim data ");

        return;
    }

    std::vector<QDateTime> sourSimTimeSteps = hdf5ReaderInterface->timeSteps();
    
    if (timeStepInfos.size() > 0)
    {
        bool isTimeStampsEqual = true;

        for (size_t i = 0; i < timeStepInfos.size(); i++)
        {
            size_t indexOnFile = timeStepIndexOnFile(i);
            if (!isEclipseAndSoursimTimeStepsEqual(timeStepInfos[i].m_date, sourSimTimeSteps[indexOnFile]))
            {
                isTimeStampsEqual = false;
            }
        }

        if (!isTimeStampsEqual) return;
    }
    else
    {
        // Use time steps from HDF to define the time steps
        QDateTime firstDate = sourSimTimeSteps[0];

        std::vector<double> daysSinceSimulationStart; 

        for (auto d : sourSimTimeSteps)
        {
            daysSinceSimulationStart.push_back(firstDate.daysTo(d));
        }

        std::vector<int> reportNumbers;
        if (m_dynamicResultsAccess.notNull())
        {
            reportNumbers = m_dynamicResultsAccess->reportNumbers();
        }
        else
        {
            for (size_t i = 0; i < sourSimTimeSteps.size(); i++)
            {
                reportNumbers.push_back(static_cast<int>(i));
            }
        }

        timeStepInfos = RigEclipseTimeStepInfo::createTimeStepInfos(sourSimTimeSteps, reportNumbers, daysSinceSimulationStart);
    }

    QStringList resultNames = hdf5ReaderInterface->propertyNames();
    for (int i = 0; i < resultNames.size(); ++i)
    {
        size_t resIndex = matrixModelResults->findOrCreateScalarResultIndex(RiaDefines::SOURSIMRL, resultNames[i], false);
        matrixModelResults->setTimeStepInfos(resIndex, timeStepInfos);
    }

    m_hdfReaderInterface = std::move(hdf5ReaderInterface);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::setFileDataAccess(RifEclipseRestartDataAccess* restartDataAccess)
{
    m_dynamicResultsAccess = restartDataAccess;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::importFaults(const QStringList& fileSet, cvf::Collection<RigFault>* faults)
{
    if (this->filenamesWithFaults().size() > 0)
    {
        for (size_t i = 0; i < this->filenamesWithFaults().size(); i++)
        {
            QString faultFilename = this->filenamesWithFaults()[i];

            RifEclipseInputFileTools::parseAndReadFaults(faultFilename, faults);
        }
    }
    else
    {
        foreach(QString fname, fileSet)
        {
            if (fname.endsWith(".DATA"))
            {
                std::vector<QString> filenamesWithFaults;
                RifEclipseInputFileTools::readFaultsInGridSection(fname, faults, &filenamesWithFaults, faultIncludeFileAbsolutePathPrefix());

                std::sort(filenamesWithFaults.begin(), filenamesWithFaults.end());
                std::vector<QString>::iterator last = std::unique(filenamesWithFaults.begin(), filenamesWithFaults.end());
                filenamesWithFaults.erase(last, filenamesWithFaults.end());

                this->setFilenamesWithFaults(filenamesWithFaults);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::transferStaticNNCData(const ecl_grid_type* mainEclGrid , ecl_file_type* init_file, RigMainGrid* mainGrid)
{
    if (!m_ecl_init_file ) return;

    CVF_ASSERT(mainEclGrid && mainGrid);

    // Get the data from ERT
    ecl_nnc_geometry_type* nnc_geo = ecl_nnc_geometry_alloc(mainEclGrid);
    ecl_nnc_data_type* tran_data = ecl_nnc_data_alloc_tran(mainEclGrid, nnc_geo, ecl_file_get_global_view(init_file));

    int numNNC = ecl_nnc_data_get_size(tran_data);
    int geometrySize = ecl_nnc_geometry_size(nnc_geo);
    CVF_ASSERT(numNNC == geometrySize);

    if (numNNC > 0)
    {
        // Transform to our own data structures

        mainGrid->nncData()->connections().resize(numNNC);
        std::vector<double>& transmissibilityValues = mainGrid->nncData()->makeStaticConnectionScalarResult(RigNNCData::propertyNameCombTrans());
        const double* transValues = ecl_nnc_data_get_values(tran_data);

        for (int nIdx = 0; nIdx < numNNC; ++nIdx)
        {
            const ecl_nnc_pair_type* geometry_pair = ecl_nnc_geometry_iget(nnc_geo, nIdx);
            RigGridBase* grid1 =  mainGrid->gridByIndex(geometry_pair->grid_nr1);
            mainGrid->nncData()->connections()[nIdx].m_c1GlobIdx = grid1->reservoirCellIndex(geometry_pair->global_index1);
            RigGridBase* grid2 =  mainGrid->gridByIndex(geometry_pair->grid_nr2);
            mainGrid->nncData()->connections()[nIdx].m_c2GlobIdx = grid2->reservoirCellIndex(geometry_pair->global_index2);

            transmissibilityValues[nIdx] = transValues[nIdx];
        }
    }

    ecl_nnc_geometry_free(nnc_geo);
    ecl_nnc_data_free(tran_data);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::transferDynamicNNCData(const ecl_grid_type* mainEclGrid, RigMainGrid* mainGrid)
{
    CVF_ASSERT(mainEclGrid && mainGrid);

    if (m_dynamicResultsAccess.isNull()) return;

    size_t timeStepCount = m_dynamicResultsAccess->timeStepCount();

    std::vector< std::vector<double> >& waterFluxData = mainGrid->nncData()->makeDynamicConnectionScalarResult(RigNNCData::propertyNameFluxWat(), timeStepCount);
    std::vector< std::vector<double> >& oilFluxData = mainGrid->nncData()->makeDynamicConnectionScalarResult(RigNNCData::propertyNameFluxOil(), timeStepCount);
    std::vector< std::vector<double> >& gasFluxData = mainGrid->nncData()->makeDynamicConnectionScalarResult(RigNNCData::propertyNameFluxGas(), timeStepCount);

    for (size_t timeStep = 0; timeStep < timeStepCount; ++timeStep)
    {
        m_dynamicResultsAccess->dynamicNNCResults(mainEclGrid, timeStep, &waterFluxData[timeStep], &oilFluxData[timeStep], &gasFluxData[timeStep]);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::openAndReadActiveCellData(const QString& fileName, const std::vector<QDateTime>& mainCaseTimeSteps, RigEclipseCaseData* eclipseCase)
{
    CVF_ASSERT(eclipseCase);

    // It is required to have a main grid before reading active cell data
    if (!eclipseCase->mainGrid())
    {
        return false;
    }

    close();

    // Get set of files
    QStringList fileSet;
    if (!RifEclipseOutputFileTools::findSiblingFilesWithSameBaseName(fileName, &fileSet)) return false;

    // Keep the set of files of interest
    m_filesWithSameBaseName = fileSet;
    m_eclipseCase = eclipseCase;


    if (!readActiveCellInfo())
    {
        return false;
    }
    
    ensureDynamicResultAccessIsPresent();
    if (m_dynamicResultsAccess.notNull())
    {
        m_dynamicResultsAccess->setTimeSteps(mainCaseTimeSteps);
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
/// See also RigStatistics::computeActiveCellUnion()
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::readActiveCellInfo()
{
    CVF_ASSERT(m_eclipseCase);
    CVF_ASSERT(m_eclipseCase->mainGrid());

    QString egridFileName = RifEclipseOutputFileTools::firstFileNameOfType(m_filesWithSameBaseName, ECL_EGRID_FILE);
    if (egridFileName.size() > 0)
    {
        ecl_file_type* ecl_file = ecl_file_open(egridFileName.toAscii().data(), ECL_FILE_CLOSE_STREAM);
        if (!ecl_file) return false;

        int actnumKeywordCount = ecl_file_get_num_named_kw(ecl_file, ACTNUM_KW);
        if (actnumKeywordCount > 0)
        {
            std::vector<std::vector<int> > actnumValuesPerGrid;
            actnumValuesPerGrid.resize(actnumKeywordCount);

            size_t reservoirCellCount = 0;
            for (size_t gridIdx = 0; gridIdx < static_cast<size_t>(actnumKeywordCount); gridIdx++)
            {
                RifEclipseOutputFileTools::keywordData(ecl_file, ACTNUM_KW, gridIdx, &actnumValuesPerGrid[gridIdx]);

                reservoirCellCount += actnumValuesPerGrid[gridIdx].size();
            }

            // Check if number of cells is matching
            if (m_eclipseCase->mainGrid()->globalCellArray().size() != reservoirCellCount)
            {
                return false;
            }

            RigActiveCellInfo* activeCellInfo = m_eclipseCase->activeCellInfo(RiaDefines::MATRIX_MODEL);
            RigActiveCellInfo* fractureActiveCellInfo = m_eclipseCase->activeCellInfo(RiaDefines::FRACTURE_MODEL);

            activeCellInfo->setReservoirCellCount(reservoirCellCount);
            fractureActiveCellInfo->setReservoirCellCount(reservoirCellCount);
            activeCellInfo->setGridCount(actnumKeywordCount);
            fractureActiveCellInfo->setGridCount(actnumKeywordCount);

            size_t cellIdx = 0;
            size_t globalActiveMatrixIndex = 0;
            size_t globalActiveFractureIndex = 0;
            for (size_t gridIdx = 0; gridIdx < static_cast<size_t>(actnumKeywordCount); gridIdx++)
            {
                size_t activeMatrixIndex = 0;
                size_t activeFractureIndex = 0;

                std::vector<int>& actnumValues = actnumValuesPerGrid[gridIdx];

                for (size_t i = 0; i < actnumValues.size(); i++)
                {
                    if (actnumValues[i] == 1 || actnumValues[i] == 3)
                    {
                        activeCellInfo->setCellResultIndex(cellIdx, globalActiveMatrixIndex++);
                        activeMatrixIndex++;
                    }

                    if (actnumValues[i] == 2 || actnumValues[i] == 3)
                    {
                        fractureActiveCellInfo->setCellResultIndex(cellIdx, globalActiveFractureIndex++);
                        activeFractureIndex++;
                    }

                    cellIdx++;
                }

                activeCellInfo->setGridActiveCellCounts(gridIdx, activeMatrixIndex);
                fractureActiveCellInfo->setGridActiveCellCounts(gridIdx, activeFractureIndex);
            }

            activeCellInfo->computeDerivedData();
            fractureActiveCellInfo->computeDerivedData();
        }

        ecl_file_close(ecl_file);

        return true;
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/// Build meta data - get states and results info
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::buildMetaData()
{
    CVF_ASSERT(m_eclipseCase);
    CVF_ASSERT(m_filesWithSameBaseName.size() > 0);

    caf::ProgressInfo progInfo(m_filesWithSameBaseName.size() + 3,"");

    progInfo.setNextProgressIncrement(m_filesWithSameBaseName.size());

    RigCaseCellResultsData* matrixModelResults = m_eclipseCase->results(RiaDefines::MATRIX_MODEL);
    RigCaseCellResultsData* fractureModelResults = m_eclipseCase->results(RiaDefines::FRACTURE_MODEL);

    std::vector<RigEclipseTimeStepInfo> timeStepInfos;

    // Create access object for dynamic results
    ensureDynamicResultAccessIsPresent();
    if (m_dynamicResultsAccess.notNull())
    {
        m_dynamicResultsAccess->open();

        progInfo.incrementProgress();

        timeStepInfos = createFilteredTimeStepInfos();

        QStringList resultNames;
        std::vector<size_t> resultNamesDataItemCounts;
        m_dynamicResultsAccess->resultNames(&resultNames, &resultNamesDataItemCounts);

        {
            QStringList matrixResultNames = validKeywordsForPorosityModel(resultNames, resultNamesDataItemCounts, 
                                                                            m_eclipseCase->activeCellInfo(RiaDefines::MATRIX_MODEL),
                                                                            m_eclipseCase->activeCellInfo(RiaDefines::FRACTURE_MODEL), 
                                                                            RiaDefines::MATRIX_MODEL, m_dynamicResultsAccess->timeStepCount());

            for (int i = 0; i < matrixResultNames.size(); ++i)
            {
                size_t resIndex = matrixModelResults->findOrCreateScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, matrixResultNames[i], false);
                matrixModelResults->setTimeStepInfos(resIndex, timeStepInfos);
            }
        }

        {
            QStringList fractureResultNames = validKeywordsForPorosityModel(resultNames, resultNamesDataItemCounts, 
                                                                            m_eclipseCase->activeCellInfo(RiaDefines::MATRIX_MODEL),
                                                                            m_eclipseCase->activeCellInfo(RiaDefines::FRACTURE_MODEL), 
                                                                            RiaDefines::FRACTURE_MODEL, m_dynamicResultsAccess->timeStepCount());

            for (int i = 0; i < fractureResultNames.size(); ++i)
            {
                size_t resIndex = fractureModelResults->findOrCreateScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, fractureResultNames[i], false);
                fractureModelResults->setTimeStepInfos(resIndex, timeStepInfos);
            }
        }

        // Default units type is METRIC
        RiaEclipseUnitTools::UnitSystem unitsType = RiaEclipseUnitTools::UNITS_METRIC;
        {
            int unitsTypeValue = m_dynamicResultsAccess->readUnitsType();
            if (unitsTypeValue == 2)
            {
                unitsType = RiaEclipseUnitTools::UNITS_FIELD;
            }
            else if (unitsTypeValue == 3)
            {
                unitsType = RiaEclipseUnitTools::UNITS_LAB;
            }
        }

        m_eclipseCase->setUnitsType(unitsType);
    }

    progInfo.incrementProgress();

    openInitFile();
    
    progInfo.incrementProgress();

    if (m_ecl_init_file)
    {
        QStringList resultNames;
        std::vector<size_t> resultNamesDataItemCounts;
        std::vector< ecl_file_type* > filesUsedToFindAvailableKeywords;
        filesUsedToFindAvailableKeywords.push_back(m_ecl_init_file);

        RifEclipseOutputFileTools::findKeywordsAndItemCount(filesUsedToFindAvailableKeywords, &resultNames, &resultNamesDataItemCounts);

        std::vector<RigEclipseTimeStepInfo> staticTimeStepInfo;
        if (!timeStepInfos.empty())
        {
            staticTimeStepInfo.push_back(timeStepInfos.front());
        }

        {
            QStringList matrixResultNames = validKeywordsForPorosityModel(resultNames, resultNamesDataItemCounts, 
                                                                            m_eclipseCase->activeCellInfo(RiaDefines::MATRIX_MODEL),
                                                                            m_eclipseCase->activeCellInfo(RiaDefines::FRACTURE_MODEL), 
                                                                            RiaDefines::MATRIX_MODEL, 1);
          
            // Add ACTNUM
            matrixResultNames += "ACTNUM";

            for (int i = 0; i < matrixResultNames.size(); ++i)
            {
                size_t resIndex = matrixModelResults->findOrCreateScalarResultIndex(RiaDefines::STATIC_NATIVE, matrixResultNames[i], false);
                matrixModelResults->setTimeStepInfos(resIndex, staticTimeStepInfo);
            }
        }

        {
            QStringList fractureResultNames = validKeywordsForPorosityModel(resultNames, resultNamesDataItemCounts, 
                                                                            m_eclipseCase->activeCellInfo(RiaDefines::MATRIX_MODEL),
                                                                            m_eclipseCase->activeCellInfo(RiaDefines::FRACTURE_MODEL), 
                                                                            RiaDefines::FRACTURE_MODEL, 1);
            // Add ACTNUM
            fractureResultNames += "ACTNUM";

            for (int i = 0; i < fractureResultNames.size(); ++i)
            {
                size_t resIndex = fractureModelResults->findOrCreateScalarResultIndex(RiaDefines::STATIC_NATIVE, fractureResultNames[i], false);
                fractureModelResults->setTimeStepInfos(resIndex, staticTimeStepInfo);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Create results access object (.UNRST or .X0001 ... .XNNNN)
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::ensureDynamicResultAccessIsPresent()
{
    if (m_dynamicResultsAccess.isNull())
    {
        m_dynamicResultsAccess = RifEclipseOutputFileTools::createDynamicResultAccess(m_fileName);
    }
}

//--------------------------------------------------------------------------------------------------
/// Get all values of a given static result as doubles
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::staticResult(const QString& result, RiaDefines::PorosityModelType matrixOrFracture, std::vector<double>* values)
{
    CVF_ASSERT(values);

    if (result.compare("ACTNUM", Qt::CaseInsensitive) == 0)
    {
        RigActiveCellInfo* activeCellInfo = m_eclipseCase->activeCellInfo(matrixOrFracture);
        values->resize(activeCellInfo->reservoirActiveCellCount(), 1.0);

        return true;
    }

    openInitFile();

    if(m_ecl_init_file)
    {
        std::vector<double> fileValues;

        size_t numOccurrences = ecl_file_get_num_named_kw(m_ecl_init_file, result.toAscii().data());
        size_t i;
        for (i = 0; i < numOccurrences; i++)
        {
            std::vector<double> partValues;
            RifEclipseOutputFileTools::keywordData(m_ecl_init_file, result, i, &partValues);
            fileValues.insert(fileValues.end(), partValues.begin(), partValues.end());
        }

        extractResultValuesBasedOnPorosityModel(matrixOrFracture, values, fileValues);
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::sourSimRlResult(const QString& result, size_t stepIndex, std::vector<double>* values)
{
    values->clear();

    if ( !m_hdfReaderInterface ) return;

    if ( m_eclipseCase->mainGrid()->gridCount() == 0 )
    {
        RiaLogging::error("No grids available");

        return ;
    }

    size_t activeCellCount = cvf::UNDEFINED_SIZE_T;
    {
        RigActiveCellInfo* fracActCellInfo = m_eclipseCase->activeCellInfo(RiaDefines::MATRIX_MODEL);
        fracActCellInfo->gridActiveCellCounts(0, activeCellCount);
    }

    size_t fileIndex = timeStepIndexOnFile(stepIndex);

    m_hdfReaderInterface->dynamicResult(result, fileIndex, values);

    if (activeCellCount != values->size())
    {
        values->clear();

        RiaLogging::error("SourSimRL results does not match the number of active cells in the grid");
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RifReaderEclipseOutput::allTimeSteps() const
{
    std::vector<QDateTime> steps;
    if (m_dynamicResultsAccess.notNull())
    {
        std::vector<double> dymmy;
        m_dynamicResultsAccess->timeSteps(&steps, &dymmy);
    }

    return steps;
}

//--------------------------------------------------------------------------------------------------
/// Get dynamic result at given step index. Will concatenate values for the main grid and all sub grids.
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::dynamicResult(const QString& result, RiaDefines::PorosityModelType matrixOrFracture, size_t stepIndex, std::vector<double>* values)
{
    ensureDynamicResultAccessIsPresent();

    if (m_dynamicResultsAccess.notNull())
    {
        size_t indexOnFile = timeStepIndexOnFile(stepIndex);

        std::vector<double> fileValues;
        if (!m_dynamicResultsAccess->results(result, indexOnFile, m_eclipseCase->mainGrid()->gridCount(), &fileValues))
        {
            return false;
        }

        extractResultValuesBasedOnPorosityModel(matrixOrFracture, values, fileValues);
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// Helper struct to store info on how a well-to-grid connection contributes to the position of 
/// well segments without any connections.
//--------------------------------------------------------------------------------------------------
struct SegmentPositionContribution
{
    SegmentPositionContribution( int         connectionSegmentId,
                                 cvf::Vec3d  connectionPosition,
                                 double      lengthFromConnection,
                                 bool        isInsolating,
                                 int         segmentIdUnder, 
                                 int         segmentIdAbove, 
                                 bool        isFromAbove)
        : m_connectionSegmentId(connectionSegmentId), 
          m_lengthFromConnection(lengthFromConnection),
          m_isInsolating(isInsolating),
          m_connectionPosition(connectionPosition),
          m_segmentIdUnder(segmentIdUnder),
          m_segmentIdAbove(segmentIdAbove),
          m_isFromAbove(isFromAbove)
    {}

    int         m_connectionSegmentId;
    double      m_lengthFromConnection;
    bool        m_isInsolating;
    cvf::Vec3d  m_connectionPosition;
    int         m_segmentIdUnder;
    int         m_segmentIdAbove;
    bool        m_isFromAbove;
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigWellResultPoint RifReaderEclipseOutput::createWellResultPoint(const RigGridBase* grid, const well_conn_type* ert_connection, int ertBranchId, int ertSegmentId, const char* wellName)
{
    CVF_ASSERT(ert_connection);
    CVF_ASSERT(grid);

    int cellI = well_conn_get_i( ert_connection );
    int cellJ = well_conn_get_j( ert_connection );
    int cellK = well_conn_get_k( ert_connection );
    bool isCellOpen = well_conn_open( ert_connection );
    double volumeRate = well_conn_get_volume_rate( ert_connection);
    double oilRate   = well_conn_get_oil_rate(ert_connection) ;
    double gasRate   = well_conn_get_gas_rate(ert_connection);
    double waterRate = well_conn_get_water_rate(ert_connection);

    // If a well is defined in fracture region, the K-value is from (cellCountK - 1) -> cellCountK*2 - 1
    // Adjust K so index is always in valid grid region
    if (cellK >= static_cast<int>(grid->cellCountK()))
    {
        cellK -= static_cast<int>(grid->cellCountK());
    }

    // See description for keyword ICON at page 54/55 of Rile Formats Reference Manual 2010.2
    /*
        Integer completion data array
        ICON(NICONZ,NCWMAX,NWELLS) with dimensions
        defined by INTEHEAD. The following items are required for each completion in each well:
        Item 1 - Well connection index ICON(1,IC,IWELL) = IC (set to -IC if connection is not in current LGR)
        Item 2 - I-coordinate (<= 0 if not in this LGR)
        Item 3 - J-coordinate (<= 0 if not in this LGR)
        Item 4 - K-coordinate (<= 0 if not in this LGR)
        Item 6 - Connection status > 0 open, <= 0 shut
        Item 14 - Penetration direction (1=x, 2=y, 3=z, 4=fractured in x-direction, 5=fractured in y-direction)
        If undefined or zero, assume Z
        Item 15 - Segment number containing connection (for multi-segment wells, =0 for ordinary wells)
        Undefined items in this array may be set to zero.
    */

    // The K value might also be -1. It is not yet known why, or what it is supposed to mean, 
    // but for now we will interpret as 0.
    // TODO: Ask Joakim Haave regarding this.
    if (cellK < 0)
    {
        //cvf::Trace::show("Well Connection for grid " + cvf::String(grid->gridName()) + "\n - Detected negative K value (K=" + cvf::String(cellK) + ") for well : " + cvf::String(wellName) + " K clamped to 0");

        cellK = 0;
    }
    
    RigWellResultPoint resultPoint;

    // Introduced based on discussion with Håkon Høgstøl 08.09.2016
    if (cellK >= static_cast<int>(grid->cellCountK()))
    {
        int maxCellK = static_cast<int>(grid->cellCountK());
        cvf::Trace::show("Well Connection for grid " + cvf::String(grid->gridName()) + "\n - Ignored connection with invalid K value (K=" + cvf::String(cellK) + ", max K = " + cvf::String(maxCellK) + ") for well : " + cvf::String(wellName));
    }
    else
    {
        resultPoint.m_gridIndex = grid->gridIndex();
        resultPoint.m_gridCellIndex = grid->cellIndexFromIJK(cellI, cellJ, cellK);

        resultPoint.m_isOpen = isCellOpen;

        resultPoint.m_ertBranchId = ertBranchId;
        resultPoint.m_ertSegmentId = ertSegmentId;
        resultPoint.m_flowRate = volumeRate; 
        resultPoint.m_oilRate   =   oilRate;
        resultPoint.m_waterRate = waterRate;

        /// Unit conversion for use with Well Allocation plots
        // Convert Gas to oil equivalents
        // If field unit, the Gas is in Mega ft^3 while the others are in [stb] (barrel) 

        // Unused Gas to Barrel conversion 
        // we convert gas to stb as well. Based on 
        // 1 [stb] = 0.15898729492800007 [m^3]
        // 1 [ft]  = 0.3048 [m]
        // megaFt3ToStbFactor = 1.0 / (1.0e-6 * 0.15898729492800007 * ( 1.0 / 0.3048 )^3 )
        // double megaFt3ToStbFactor = 178107.60668;
        
        double fieldGasToOilEquivalent  = 1.0e6/5800; // Mega ft^3 to BOE
        double metricGasToOilEquivalent = 1.0/1.0e3; // Sm^3 Gas to Sm^3 oe  

        if (m_eclipseCase->unitsType() == RiaEclipseUnitTools::UNITS_FIELD)  gasRate = fieldGasToOilEquivalent * gasRate; 
        if (m_eclipseCase->unitsType() == RiaEclipseUnitTools::UNITS_METRIC) gasRate = metricGasToOilEquivalent * gasRate; 

        resultPoint.m_gasRate   =   gasRate;
    }

    return resultPoint;
}


//--------------------------------------------------------------------------------------------------
/// Inverse distance interpolation of the supplied points and distance weights for 
/// the contributing points which are closest above, and closest below
//--------------------------------------------------------------------------------------------------
cvf::Vec3d interpolate3DPosition(const std::vector<SegmentPositionContribution>& positions)
{
    std::vector<SegmentPositionContribution> filteredPositions;
    filteredPositions.reserve(positions.size());

    double minDistFromContribAbove = HUGE_VAL;
    double minDistFromContribBelow = HUGE_VAL;
    std::vector<SegmentPositionContribution> contrFromAbove;
    std::vector<SegmentPositionContribution> contrFromBelow;


    for (size_t i = 0; i < positions.size(); i++)
    {
        if (positions[i].m_connectionPosition != cvf::Vec3d::UNDEFINED)
        {
            if (positions[i].m_isFromAbove && positions[i].m_lengthFromConnection < minDistFromContribAbove)
            {
                if (contrFromAbove.size()) contrFromAbove[0] = positions[i];
                else contrFromAbove.push_back(positions[i]);

                minDistFromContribAbove = positions[i].m_lengthFromConnection;
            }

            if (! positions[i].m_isFromAbove && positions[i].m_lengthFromConnection < minDistFromContribBelow)
            {
                if (contrFromBelow.size()) contrFromBelow[0] = positions[i];
                else contrFromBelow.push_back(positions[i]);

                minDistFromContribBelow = positions[i].m_lengthFromConnection;

            }
        }
    }

    filteredPositions = contrFromAbove;
    filteredPositions.insert(filteredPositions.end(), contrFromBelow.begin(), contrFromBelow.end());

    std::vector<double> nominators(filteredPositions.size(), 0.0);

    double denominator = 0.0;
    cvf::Vec3d interpolatedValue = cvf::Vec3d::ZERO;

    for (size_t i = 0; i < filteredPositions.size(); i++)
    {
#if 0 // Pure average test
        nominators[i] = 1.0;
#else
        double distance = filteredPositions[i].m_lengthFromConnection;

        if (distance < 1e-6)
        {
            return filteredPositions[i].m_connectionPosition;
        }
        else if (distance < 1.0)
        {
            //distance = 1.0;
        }


        distance = 1.0 / distance;
        nominators[i] = distance;
        denominator  += distance;

#endif
    }
#if 0 // Pure average test
    denominator = positions.size(); // Pure average test
#endif
    for (size_t i = 0; i < filteredPositions.size(); i++)
    {
        interpolatedValue += (nominators[i]/denominator) * filteredPositions[i].m_connectionPosition;
    }

    return interpolatedValue;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void propagatePosContribDownwards(std::map<int, std::vector<SegmentPositionContribution> > & segmentIdToPositionContrib,
    const well_segment_collection_type * allErtSegments,
    int ertSegmentId, 
    std::vector<SegmentPositionContribution> posContrib)
{

    std::map<int, std::vector<SegmentPositionContribution> >::iterator posContribIt;
    posContribIt = segmentIdToPositionContrib.find(ertSegmentId);


    if ( posContribIt != segmentIdToPositionContrib.end())
    {
        // Create a set of the segments below this, that has to be followed.

        std::set<int> segmentIdsBelow;
        for (size_t i = 0 ; i < posContribIt->second.size(); ++i)
        {
            segmentIdsBelow.insert(posContribIt->second[i].m_segmentIdUnder);
        }

        // Get the segment length to add to the contributions

        well_segment_type  *segment       = well_segment_collection_get( allErtSegments , posContribIt->first); 
        double sementLength               = well_segment_get_length(segment);

        // If we do not have the contribution represented, add it, and accumulate the length
        // If it is already present, do not touch
        for (size_t i = 0; i < posContrib.size(); ++i)
        {
            bool foundContribution = false;
            for (size_t j = 0 ; j < posContribIt->second.size(); ++j)
            {
                if (posContribIt->second[j].m_connectionSegmentId == posContrib[i].m_connectionSegmentId)
                {
                    foundContribution = true;
                    break;
                }
            }

            if (! foundContribution)
            {
                posContrib[i].m_lengthFromConnection += sementLength;
                posContrib[i].m_isFromAbove = true;
                posContribIt->second.push_back(posContrib[i]);
            }
            posContrib[i].m_segmentIdAbove = ertSegmentId;
        }

        for (std::set<int>::iterator it = segmentIdsBelow.begin(); it != segmentIdsBelow.end(); ++it)
        {
            propagatePosContribDownwards(segmentIdToPositionContrib, allErtSegments, (*it), posContrib);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::readWellCells(const ecl_grid_type* mainEclGrid, bool importCompleteMswData)
{
    CVF_ASSERT(m_eclipseCase);

    if (m_dynamicResultsAccess.isNull()) return;

    well_info_type* ert_well_info = well_info_alloc(mainEclGrid);
    if (!ert_well_info) return;

    m_dynamicResultsAccess->readWellData(ert_well_info, importCompleteMswData);

    std::vector<double> daysSinceSimulationStart;
    std::vector<QDateTime> timeSteps;
    m_dynamicResultsAccess->timeSteps(&timeSteps, &daysSinceSimulationStart);
    std::vector<int> reportNumbers = m_dynamicResultsAccess->reportNumbers();

    bool sameCount = false;
    if (timeSteps.size() == reportNumbers.size())
    {
        sameCount = true;
    }

    std::vector<RigGridBase*> grids;
    m_eclipseCase->allGrids(&grids);

    cvf::Collection<RigSingleWellResultsData> wells;
    caf::ProgressInfo progress(well_info_get_num_wells(ert_well_info), "");

    int wellIdx;
    for (wellIdx = 0; wellIdx < well_info_get_num_wells(ert_well_info); wellIdx++)
    {
        const char* wellName = well_info_iget_well_name(ert_well_info, wellIdx);
        CVF_ASSERT(wellName);

        cvf::ref<RigSingleWellResultsData> wellResults = new RigSingleWellResultsData;
        wellResults->m_wellName = wellName;

        well_ts_type* ert_well_time_series = well_info_get_ts(ert_well_info , wellName);
        int timeStepCount = well_ts_get_size(ert_well_time_series);

        wellResults->m_wellCellsTimeSteps.resize(timeStepCount);

        int timeIdx;
        for (timeIdx = 0; timeIdx < timeStepCount; timeIdx++)
        {
            well_state_type* ert_well_state = well_ts_iget_state(ert_well_time_series, timeIdx);

            RigWellResultFrame& wellResFrame = wellResults->m_wellCellsTimeSteps[timeIdx];

            // Build timestamp for well
            bool haveFoundTimeStamp = false;
            
            if (sameCount)
            {
                int reportNr = well_state_get_report_nr(ert_well_state);

                for (size_t i = 0; i < reportNumbers.size(); i++)
                {
                    if (reportNumbers[i] == reportNr)
                    {
                        wellResFrame.m_timestamp = timeSteps[i];
                        haveFoundTimeStamp = true;
                    }
                }
            }

            if (!haveFoundTimeStamp)
            {
                // This fallback will not work for timesteps before 1970.

                // Also see RifEclipseOutputFileAccess::timeStepsText for accessing time_t structures
                time_t stepTime = well_state_get_sim_time(ert_well_state);
                wellResFrame.m_timestamp = QDateTime::fromTime_t(stepTime);
            }

            // Production type
            well_type_enum ert_well_type = well_state_get_type(ert_well_state);
            if (ert_well_type == ERT_PRODUCER)
            {
                wellResFrame.m_productionType = RigWellResultFrame::PRODUCER;
            }
            else if (ert_well_type == ERT_WATER_INJECTOR)
            {
                wellResFrame.m_productionType = RigWellResultFrame::WATER_INJECTOR;
            }
            else if (ert_well_type == ERT_GAS_INJECTOR)
            {
                wellResFrame.m_productionType = RigWellResultFrame::GAS_INJECTOR;
            }
            else if (ert_well_type == ERT_OIL_INJECTOR)
            {
                wellResFrame.m_productionType = RigWellResultFrame::OIL_INJECTOR;
            }
            else
            {
                wellResFrame.m_productionType = RigWellResultFrame::UNDEFINED_PRODUCTION_TYPE;
            }

            wellResFrame.m_isOpen = well_state_is_open( ert_well_state );


            if (importCompleteMswData && well_state_is_MSW(ert_well_state))
            {
                wellResults->setMultiSegmentWell(true);

                // how do we handle LGR-s ? 
                // 1. Create separate visual branches for each Grid, with its own wellhead
                // 2. Always use the connections to the grid with the highest number (innermost LGR). 
                // 3. Handle both and switch between them according to visual settings of grid visualization
                // Will there ever exist connections to different grids for the same segment ?
                // We have currently selected 2.

                // Set the wellhead

                int lastGridNr = static_cast<int>(grids.size()) - 1;
                for (int gridNr = lastGridNr; gridNr >= 0; --gridNr)
                {
                    //  If several grids have a wellhead definition for this well, we use the last one. 
                    // (Possibly the innermost LGR)

                    const well_conn_type* ert_wellhead = well_state_iget_wellhead(ert_well_state, static_cast<int>(gridNr));
                    if (ert_wellhead)
                    {
                        wellResFrame.m_wellHead = createWellResultPoint(grids[gridNr], ert_wellhead, -1, -1, wellName);

                        // HACK: Ert returns open as "this is equally wrong as closed for well heads". 
                        // Well heads are not open jfr mail communication with HHGS and JH Statoil 07.01.2016
                        wellResFrame.m_wellHead.m_isOpen = false; 
                        break;
                    }
                }

                well_branch_collection_type* branches = well_state_get_branches(ert_well_state);
                int branchCount = well_branch_collection_get_size(branches);
                wellResFrame.m_wellResultBranches.resize( branchCount);
                std::map<int, std::vector<SegmentPositionContribution> > segmentIdToPositionContrib;
                std::vector<int> upperSegmentIdsOfUnpositionedSegementGroup;

                // For each branch, go from bottom segment upwards and transfer their connections to WellResultpoints.
                // If they have no connections, create a resultpoint representing their bottom position, which will 
                // receive an actual position at a later stage.
                // I addition, distribute contributions for calculating segment bottom positions from bottom and up.


                for (int bIdx = 0; bIdx < well_branch_collection_get_size(branches); bIdx++)
                {
                    RigWellResultBranch& wellResultBranch = wellResFrame.m_wellResultBranches[ bIdx];

                    const well_segment_type* segment = well_branch_collection_iget_start_segment(branches, bIdx);

                    int branchId = well_segment_get_branch_id(segment);
                    wellResultBranch.m_ertBranchId = branchId;

                    // Data for segment position calculation
                    int lastConnectionSegmentId        = -1;
                    cvf::Vec3d lastConnectionPos       = cvf::Vec3d::UNDEFINED;
                    cvf::Vec3d lastConnectionCellCorner= cvf::Vec3d::UNDEFINED;
                    double lastConnectionCellSize      = 0;
                    double accLengthFromLastConnection = 0;
                    int segmentIdBelow                 = -1;
                    bool segmentBelowHasConnections    = false;

                    while (segment && branchId == well_segment_get_branch_id(segment))
                    {
                        // Loop backwards, making us select the connection in the innermost lgr as the truth
                        bool segmentHasConnections = false;

                        for (int gridNr = lastGridNr; gridNr >= 0; --gridNr)
                        {
                            std::string gridName = this->ertGridName(gridNr);

                            // If this segment has connections in any grid, transfer the innermost ones

                            if (well_segment_has_grid_connections(segment, gridName.data()))
                            {
                                const well_conn_collection_type* connections = well_segment_get_connections(segment, gridName.data());
                                int connectionCount = well_conn_collection_get_size(connections);

                                // Loop backwards to put the deepest connections first in the array. (The segments are also traversed deep to shallow)
                                for (int connIdx = connectionCount-1; connIdx >= 0; connIdx--)
                                {
                                    well_conn_type* ert_connection = well_conn_collection_iget(connections, connIdx);
                                    wellResultBranch.m_branchResultPoints.push_back(
                                        createWellResultPoint(grids[gridNr], ert_connection, branchId, well_segment_get_id(segment), wellName));
                                }

                                segmentHasConnections = true;

                                // Prepare data for segment position calculation

                                well_conn_type* ert_connection = well_conn_collection_iget(connections, 0);
                                RigWellResultPoint point       = createWellResultPoint(grids[gridNr], ert_connection, branchId, well_segment_get_id(segment), wellName);
                                lastConnectionPos              = grids[gridNr]->cell(point.m_gridCellIndex).center();
                                cvf::Vec3d cellVxes[8];
                                grids[gridNr]->cellCornerVertices(point.m_gridCellIndex, cellVxes);
                                lastConnectionCellCorner       = cellVxes[0];
                                lastConnectionCellSize         = (lastConnectionPos - cellVxes[0]).length();


                                lastConnectionSegmentId        = well_segment_get_id(segment);
                                accLengthFromLastConnection    = well_segment_get_length(segment)/(connectionCount+1);
                                if ( ! segmentBelowHasConnections) upperSegmentIdsOfUnpositionedSegementGroup.push_back(segmentIdBelow);

                                break; // Stop looping over grids
                            }
                        }
                        
                        // If the segment did not have connections at all, we need to create a resultpoint representing the bottom of the segment
                        // and store it as an unpositioned segment

                        if (!segmentHasConnections)
                        {
                            RigWellResultPoint data;
                            data.m_ertBranchId = branchId;
                            data.m_ertSegmentId = well_segment_get_id(segment);

                            wellResultBranch.m_branchResultPoints.push_back(data);

                            // Store data for segment position calculation
                            bool isAnInsolationContribution = accLengthFromLastConnection < lastConnectionCellSize;

                          
                            segmentIdToPositionContrib[well_segment_get_id(segment)].push_back( 
                                SegmentPositionContribution(lastConnectionSegmentId, lastConnectionPos, accLengthFromLastConnection, isAnInsolationContribution, segmentIdBelow, -1, false));
                            accLengthFromLastConnection += well_segment_get_length(segment);
                           
                        }

                        segmentIdBelow = well_segment_get_id(segment);
                        segmentBelowHasConnections = segmentHasConnections;

                        if (well_segment_get_outlet_id(segment) == -1)
                        {
                            segment = NULL;
                        }
                        else
                        {
                            segment = well_segment_get_outlet(segment);
                        }
                    }

                    // Add resultpoint representing the outlet segment (bottom), if not the branch ends at the wellhead.

                    const well_segment_type* outletSegment = segment;

                    if (outletSegment)
                    {
                        bool outletSegmentHasConnections = false;

                        for (int gridNr = lastGridNr; gridNr >= 0; --gridNr)
                        {
                            std::string gridName = this->ertGridName(gridNr);

                            // If this segment has connections in any grid, use the deepest innermost one

                            if (well_segment_has_grid_connections(outletSegment, gridName.data()))
                            {
                                const well_conn_collection_type* connections = well_segment_get_connections(outletSegment, gridName.data());
                                int connectionCount = well_conn_collection_get_size(connections);

                                // Select the deepest connection 
                                well_conn_type* ert_connection = well_conn_collection_iget(connections, connectionCount-1);
                                wellResultBranch.m_branchResultPoints.push_back(
                                    createWellResultPoint(grids[gridNr], ert_connection, branchId, well_segment_get_id(outletSegment), wellName));

                                outletSegmentHasConnections = true;
                                break; // Stop looping over grids
                            }
                        }

                        if (!outletSegmentHasConnections)
                        {
                            // Store the result point

                            RigWellResultPoint data;
                            data.m_ertBranchId = well_segment_get_branch_id(outletSegment);
                            data.m_ertSegmentId = well_segment_get_id(outletSegment);
                            wellResultBranch.m_branchResultPoints.push_back(data);

                            // Store data for segment position calculation, 
                            // and propagate it upwards until we meet a segment with connections

                            bool isAnInsolationContribution = accLengthFromLastConnection < lastConnectionCellSize;

                            cvf::Vec3d lastConnectionPosWOffset = lastConnectionPos;
                            if (isAnInsolationContribution) lastConnectionPosWOffset += 0.4*(lastConnectionCellCorner-lastConnectionPos);

                            segmentIdToPositionContrib[well_segment_get_id(outletSegment)].push_back( 
                                SegmentPositionContribution(lastConnectionSegmentId, lastConnectionPosWOffset, accLengthFromLastConnection, isAnInsolationContribution, segmentIdBelow, -1, false));

                            /// Loop further to add this position contribution until a segment with connections is found

                            accLengthFromLastConnection += well_segment_get_length(outletSegment);
                            segmentIdBelow = well_segment_get_id(outletSegment);

                            const well_segment_type* aboveOutletSegment = NULL;

                            if (well_segment_get_outlet_id(outletSegment) == -1)
                            {
                                aboveOutletSegment = NULL;
                            }
                            else
                            {
                                aboveOutletSegment = well_segment_get_outlet(outletSegment);
                            }

                            while (aboveOutletSegment )
                            {
                                // Loop backwards, just because we do that the other places
                                bool segmentHasConnections = false;

                                for (int gridNr = lastGridNr; gridNr >= 0; --gridNr)
                                {
                                    std::string gridName = this->ertGridName(gridNr);

                                    // If this segment has connections in any grid, stop traversal

                                    if (well_segment_has_grid_connections(aboveOutletSegment, gridName.data()))
                                    {
                                        segmentHasConnections = true;
                                        break;
                                    }
                                }

                                if (!segmentHasConnections)
                                {
                                    segmentIdToPositionContrib[well_segment_get_id(aboveOutletSegment)].push_back( 
                                        SegmentPositionContribution(lastConnectionSegmentId, lastConnectionPos, accLengthFromLastConnection, isAnInsolationContribution, segmentIdBelow, -1, false));
                                    accLengthFromLastConnection += well_segment_get_length(aboveOutletSegment);
                                }
                                else
                                {
                                    break; // We have found a segment with connections. We do not need to propagate position contributions further
                                }

                                segmentIdBelow = well_segment_get_id(aboveOutletSegment);

                                if (well_segment_get_outlet_id(aboveOutletSegment) == -1)
                                {
                                    aboveOutletSegment = NULL;
                                }
                                else
                                {
                                    aboveOutletSegment = well_segment_get_outlet(aboveOutletSegment);
                                }
                            }


                        }
                    }
                    else
                    {
                        // Add wellhead as result point Nope. Not Yet, but it is a good idea. 
                        // The centerline calculations would be a bit simpler, I think.
                    }

                    // Reverse the order of the resultpoints in this branch, making the deepest come last

                    std::reverse(wellResultBranch.m_branchResultPoints.begin(), wellResultBranch.m_branchResultPoints.end());
                } // End of the branch loop


                // Propagate position contributions from connections above unpositioned segments downwards

                well_segment_collection_type * allErtSegments = well_state_get_segments( ert_well_state );

                for (size_t bIdx = 0; bIdx <  wellResFrame.m_wellResultBranches.size(); ++bIdx)
                {
                    RigWellResultBranch& wellResultBranch = wellResFrame.m_wellResultBranches[ bIdx];
                    bool previousResultPointWasCell = false;
                    if (bIdx == 0) previousResultPointWasCell = true; // Wellhead 

                    // Go downwards until we find a none-cell resultpoint just after a cell-resultpoint
                    // When we do, start propagating

                    for (size_t rpIdx = 0; rpIdx < wellResultBranch.m_branchResultPoints.size(); ++rpIdx)
                    {
                        RigWellResultPoint resPoint = wellResultBranch.m_branchResultPoints[rpIdx];
                        if ( resPoint.isCell() )
                        {
                            previousResultPointWasCell = true;
                        }
                        else
                        {
                            if (previousResultPointWasCell)
                            {
                                RigWellResultPoint prevResPoint;
                                if (bIdx == 0 && rpIdx == 0)
                                {
                                    prevResPoint            = wellResFrame.m_wellHead;
                                }
                                else
                                {
                                    prevResPoint            = wellResultBranch.m_branchResultPoints[rpIdx - 1 ];
                                }

                                cvf::Vec3d         lastConnectionPos   = grids[prevResPoint.m_gridIndex]->cell(prevResPoint.m_gridCellIndex).center();

                                SegmentPositionContribution posContrib(prevResPoint.m_ertSegmentId, lastConnectionPos, 0.0, false,  -1, prevResPoint.m_ertSegmentId, true);

                                int ertSegmentId = resPoint.m_ertSegmentId;

                                std::map<int, std::vector<SegmentPositionContribution> >::iterator posContribIt;
                                posContribIt = segmentIdToPositionContrib.find(ertSegmentId);
                                CVF_ASSERT(posContribIt != segmentIdToPositionContrib.end());

                                std::vector<SegmentPositionContribution> posContributions =  posContribIt->second;
                                for (size_t i = 0; i < posContributions.size(); ++i)
                                {
                                    posContributions[i].m_segmentIdAbove = prevResPoint.m_ertSegmentId;
                                }
                                posContributions.push_back(posContrib);

                                propagatePosContribDownwards(segmentIdToPositionContrib, allErtSegments, ertSegmentId, posContributions);
                            }

                            previousResultPointWasCell = false;
                        }
                    }
                }

                // Calculate the bottom position of all the unpositioned segments
                // Then do the calculation based on the refined contributions

                std::map<int, std::vector<SegmentPositionContribution> >::iterator posContribIt = segmentIdToPositionContrib.begin();
                std::map<int, cvf::Vec3d> bottomPositions;
                while (posContribIt != segmentIdToPositionContrib.end())
                {
                    bottomPositions[posContribIt->first] = interpolate3DPosition(posContribIt->second);
                    ++posContribIt;
                }

                // Distribute the positions to the resultpoints stored in the wellResultBranch.m_branchResultPoints

                for (size_t bIdx = 0; bIdx <  wellResFrame.m_wellResultBranches.size(); ++bIdx)
                {
                    RigWellResultBranch& wellResultBranch = wellResFrame.m_wellResultBranches[ bIdx];
                    for (size_t rpIdx = 0; rpIdx < wellResultBranch.m_branchResultPoints.size(); ++rpIdx)
                    {
                        RigWellResultPoint & resPoint = wellResultBranch.m_branchResultPoints[rpIdx];
                        if ( ! resPoint.isCell() )
                        {
                            resPoint.m_bottomPosition = bottomPositions[resPoint.m_ertSegmentId];
                        }
                    }
                }

            } // End of the MSW section
            else 
            {
                // Code handling None-MSW Wells ... Normal wells that is.

                // Loop over all the grids in the model. If we have connections in one, we will discard
                // the main grid connections as the well connections are duplicated in the main grid and LGR grids 
                // Verified on 10 k case JJS. But smarter things could be done, like showing the "main grid well" if turning off the LGR's

                bool hasWellConnectionsInLGR = false;

                for (size_t gridIdx = 1; gridIdx < grids.size(); ++gridIdx)
                {
                    RigGridBase* lgrGrid = m_eclipseCase->grid(gridIdx);
                    if (well_state_has_grid_connections(ert_well_state, lgrGrid->gridName().data()))
                    {
                        hasWellConnectionsInLGR = true;
                        break;
                    }
                }

                size_t gridNr = hasWellConnectionsInLGR ? 1 : 0;
                for (; gridNr < grids.size(); ++gridNr)
                {

                    // Wellhead. If several grids have a wellhead definition for this well, we use the last one. (Possibly the innermost LGR)
                    const well_conn_type* ert_wellhead = well_state_iget_wellhead(ert_well_state, static_cast<int>(gridNr));
                    if (ert_wellhead)
                    {
                        wellResFrame.m_wellHead = createWellResultPoint(grids[gridNr], ert_wellhead, -1, -1, wellName);
                        // HACK: Ert returns open as "this is equally wrong as closed for well heads". 
                        // Well heads are not open jfr mail communication with HHGS and JH Statoil 07.01.2016
                        wellResFrame.m_wellHead.m_isOpen = false; 

                        //std::cout << "Wellhead YES at timeIdx: " << timeIdx <<  " wellIdx: " << wellIdx << " Grid: " << gridNr << std::endl;
                    }
                    else
                    {
                        // std::cout << "Wellhead NO  at timeIdx: " << timeIdx <<  " wellIdx: " << wellIdx << " Grid: " << gridNr << std::endl;
                        //CVF_ASSERT(0); // This is just a test assert to see if this condition exists in some files and it does.
                        // All the grids does not necessarily have a well head definition. 
                    }

                    const well_conn_collection_type* connections = well_state_get_grid_connections(ert_well_state, this->ertGridName(gridNr).data());

                    // Import all well result cells for all connections
                    if (connections)
                    {
                        int connectionCount = well_conn_collection_get_size(connections);
                        if (connectionCount)
                        {
                            wellResFrame.m_wellResultBranches.push_back(RigWellResultBranch());
                            RigWellResultBranch& wellResultBranch = wellResFrame.m_wellResultBranches.back();

                            wellResultBranch.m_ertBranchId = 0; // Normal wells have only one branch

                            size_t existingCellCount = wellResultBranch.m_branchResultPoints.size();
                            wellResultBranch.m_branchResultPoints.resize(existingCellCount + connectionCount);

                            for (int connIdx = 0; connIdx < connectionCount; connIdx++)
                            {
                                well_conn_type* ert_connection = well_conn_collection_iget(connections, connIdx);
                                wellResultBranch.m_branchResultPoints[existingCellCount + connIdx] = 
                                    createWellResultPoint(grids[gridNr], ert_connection, -1, -1, wellName);
                            }
                        }
                    }
                }
            }
        }


        std::vector<QDateTime> filteredTimeSteps;
        {
            std::vector<RigEclipseTimeStepInfo> filteredTimeStepInfos = createFilteredTimeStepInfos();
            for (auto a : filteredTimeStepInfos)
            {
                filteredTimeSteps.push_back(a.m_date);
            }
        }

        wellResults->computeMappingFromResultTimeIndicesToWellTimeIndices(filteredTimeSteps);

        wells.push_back(wellResults.p());

        progress.incrementProgress();
    }

    well_info_free(ert_well_info);

    m_eclipseCase->setWellResults(wells);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RifReaderEclipseOutput::validKeywordsForPorosityModel(const QStringList& keywords, 
                                                                  const std::vector<size_t>& keywordDataItemCounts, 
                                                                  const RigActiveCellInfo* matrixActiveCellInfo, 
                                                                  const RigActiveCellInfo* fractureActiveCellInfo, 
                                                                  RiaDefines::PorosityModelType porosityModel, 
                                                                  size_t timeStepCount) const
{
    CVF_ASSERT(matrixActiveCellInfo);

    if (keywords.size() != static_cast<int>(keywordDataItemCounts.size()))
    {
        return QStringList();
    }

    if (porosityModel == RiaDefines::FRACTURE_MODEL)
    {
        if (fractureActiveCellInfo->reservoirActiveCellCount() == 0)
        {
            return QStringList();
        }
    }

    QStringList keywordsWithCorrectNumberOfDataItems;
    
    for (int i = 0; i < keywords.size(); i++)
    {
        QString keyword = keywords[i];
        size_t keywordDataItemCount = keywordDataItemCounts[i];

        bool validKeyword = false;

        size_t timeStepsAllCellsRest = keywordDataItemCount % matrixActiveCellInfo->reservoirCellCount();
        if (timeStepsAllCellsRest == 0 && keywordDataItemCount <= timeStepCount * matrixActiveCellInfo->reservoirCellCount())
        {
            // Found result for all cells for N time steps, usually a static dataset for one time step
            validKeyword = true;
        }
        else 
        {
            size_t timeStepsMatrixRest = keywordDataItemCount % matrixActiveCellInfo->reservoirActiveCellCount();

            size_t timeStepsFractureRest = 0;
            if (fractureActiveCellInfo->reservoirActiveCellCount() > 0)
            {
                timeStepsFractureRest = keywordDataItemCount % fractureActiveCellInfo->reservoirActiveCellCount();
            }

            size_t sumFractureMatrixActiveCellCount = matrixActiveCellInfo->reservoirActiveCellCount() + fractureActiveCellInfo->reservoirActiveCellCount();
            size_t timeStepsMatrixAndFractureRest = keywordDataItemCount % sumFractureMatrixActiveCellCount;

            if (porosityModel == RiaDefines::MATRIX_MODEL && timeStepsMatrixRest == 0)
            {
                if (keywordDataItemCount <= timeStepCount * std::max(matrixActiveCellInfo->reservoirActiveCellCount(), sumFractureMatrixActiveCellCount))
                {
                    validKeyword = true;
                }
            }
            else if (porosityModel == RiaDefines::FRACTURE_MODEL && fractureActiveCellInfo->reservoirActiveCellCount() > 0 && timeStepsFractureRest == 0)
            {
                if (keywordDataItemCount <= timeStepCount * std::max(fractureActiveCellInfo->reservoirActiveCellCount(), sumFractureMatrixActiveCellCount))
                {
                    validKeyword = true;
                }
            }
            else if (timeStepsMatrixAndFractureRest == 0)
            {
                if (keywordDataItemCount <= timeStepCount * sumFractureMatrixActiveCellCount)
                {
                    validKeyword = true;
                }
            }
        }

        // Check for INIT values that has only values for main grid active cells
        if (!validKeyword)
        {
            if (timeStepCount == 1)
            {
                size_t mainGridMatrixActiveCellCount;   matrixActiveCellInfo->gridActiveCellCounts(0, mainGridMatrixActiveCellCount);
                size_t mainGridFractureActiveCellCount; fractureActiveCellInfo->gridActiveCellCounts(0, mainGridFractureActiveCellCount);

                if (   keywordDataItemCount == mainGridMatrixActiveCellCount 
                    || keywordDataItemCount == mainGridFractureActiveCellCount 
                    || keywordDataItemCount == mainGridMatrixActiveCellCount + mainGridFractureActiveCellCount )
                {
                    validKeyword = true;
                }
            }
        }

        if (validKeyword)
        {
            keywordsWithCorrectNumberOfDataItems.push_back(keyword);
        }
    }

    return keywordsWithCorrectNumberOfDataItems;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RigEclipseTimeStepInfo> RifReaderEclipseOutput::createFilteredTimeStepInfos()
{
    std::vector<RigEclipseTimeStepInfo> timeStepInfos;

    if (m_dynamicResultsAccess.notNull())
    {
        std::vector<QDateTime> timeStepsOnFile;
        std::vector<double> daysSinceSimulationStartOnFile;
        std::vector<int> reportNumbersOnFile;

        m_dynamicResultsAccess->timeSteps(&timeStepsOnFile, &daysSinceSimulationStartOnFile);
        reportNumbersOnFile = m_dynamicResultsAccess->reportNumbers();

        for (size_t i = 0; i < timeStepsOnFile.size(); i++)
        {
            if (this->isTimeStepIncludedByFilter(i))
            {
                timeStepInfos.push_back(RigEclipseTimeStepInfo(timeStepsOnFile[i], reportNumbersOnFile[i], daysSinceSimulationStartOnFile[i]));
            }
        }
    }

    return timeStepInfos;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::isEclipseAndSoursimTimeStepsEqual(const QDateTime& eclipseDateTime, const QDateTime& sourSimDateTime)
{
    // Compare date down to and including seconds
    // Compare of complete date time objects will often result in differences

    const int secondsThreshold = 4;
    const QString dateStr("yyyy.MMM.dd hh:mm:ss:zzz");

    int secondsDiff = eclipseDateTime.secsTo(sourSimDateTime);
    if (secondsDiff > secondsThreshold)
    {
        RiaLogging::error("HDF: Time steps does not match");

        RiaLogging::error(QString("  %1 - Eclipse").arg(eclipseDateTime.toString(dateStr)));
        RiaLogging::error(QString("  %1 - SourSim").arg(sourSimDateTime.toString(dateStr)));

        return false;
    }

    if (eclipseDateTime.time().second() != sourSimDateTime.time().second())
    {
        RiaLogging::warning("HDF: Time steps differ, but within time step compare threshold");
        RiaLogging::warning(QString("  %1 - Eclipse").arg(eclipseDateTime.toString(dateStr)));
        RiaLogging::warning(QString("  %1 - SourSim").arg(sourSimDateTime.toString(dateStr)));
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::extractResultValuesBasedOnPorosityModel(RiaDefines::PorosityModelType matrixOrFracture, std::vector<double>* destinationResultValues, const std::vector<double>& sourceResultValues)
{
    if (sourceResultValues.size() == 0) return;

    RigActiveCellInfo* fracActCellInfo = m_eclipseCase->activeCellInfo(RiaDefines::FRACTURE_MODEL); 

    if (matrixOrFracture == RiaDefines::MATRIX_MODEL && fracActCellInfo->reservoirActiveCellCount() == 0)
    {
        destinationResultValues->insert(destinationResultValues->end(), sourceResultValues.begin(), sourceResultValues.end());
    }
    else
    {
        RigActiveCellInfo* actCellInfo = m_eclipseCase->activeCellInfo(RiaDefines::MATRIX_MODEL); 

        size_t sourceStartPosition = 0;

        for (size_t i = 0; i < m_eclipseCase->mainGrid()->gridCount(); i++)
        {
            size_t matrixActiveCellCount = 0;
            size_t fractureActiveCellCount = 0;
           
            actCellInfo->gridActiveCellCounts(i, matrixActiveCellCount);
            fracActCellInfo->gridActiveCellCounts(i, fractureActiveCellCount);

            if (matrixOrFracture == RiaDefines::MATRIX_MODEL)
            {
                destinationResultValues->insert(destinationResultValues->end(), 
                                                sourceResultValues.begin() + sourceStartPosition, 
                                                sourceResultValues.begin() + sourceStartPosition + matrixActiveCellCount);
            }
            else
            {
                destinationResultValues->insert(destinationResultValues->end(), 
                                                sourceResultValues.begin() + sourceStartPosition + matrixActiveCellCount, 
                                                sourceResultValues.begin() + sourceStartPosition + matrixActiveCellCount + fractureActiveCellCount);
            }

            sourceStartPosition += (matrixActiveCellCount + fractureActiveCellCount);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::openInitFile()
{
    if (m_ecl_init_file)
    {
        return;
    }

    QString initFileName = RifEclipseOutputFileTools::firstFileNameOfType(m_filesWithSameBaseName, ECL_INIT_FILE);
    if (initFileName.size() > 0)
    {
        m_ecl_init_file = ecl_file_open(initFileName.toAscii().data(), ECL_FILE_CLOSE_STREAM);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::transferCoarseningInfo(const ecl_grid_type* eclGrid, RigGridBase* grid)
{
    int coarseGroupCount = ecl_grid_get_num_coarse_groups(eclGrid);
    for (int i = 0; i < coarseGroupCount; i++)
    {
        ecl_coarse_cell_type* coarse_cell = ecl_grid_iget_coarse_group(eclGrid, i);
        CVF_ASSERT(coarse_cell);

        size_t i1 = static_cast<size_t>(ecl_coarse_cell_get_i1(coarse_cell));
        size_t i2 = static_cast<size_t>(ecl_coarse_cell_get_i2(coarse_cell));
        size_t j1 = static_cast<size_t>(ecl_coarse_cell_get_j1(coarse_cell));
        size_t j2 = static_cast<size_t>(ecl_coarse_cell_get_j2(coarse_cell));
        size_t k1 = static_cast<size_t>(ecl_coarse_cell_get_k1(coarse_cell));
        size_t k2 = static_cast<size_t>(ecl_coarse_cell_get_k2(coarse_cell));

        grid->addCoarseningBox(i1, i2, j1, j2, k1, k2);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RifReaderEclipseOutput::ertGridName(size_t gridNr)
{
    std::string gridName;
    if (gridNr == 0)
    {
        gridName = ECL_GRID_GLOBAL_GRID;
    }
    else
    {
        CVF_ASSERT(m_eclipseCase);
        CVF_ASSERT(m_eclipseCase->gridCount() > gridNr);
        gridName = m_eclipseCase->grid(gridNr)->gridName();
    }

    return gridName;
}

