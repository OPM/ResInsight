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

#include "cvfBase.h"

#include "RigMainGrid.h"
#include "RigEclipseCase.h"
#include "RigReservoirCellResults.h"

#include "RifReaderEclipseOutput.h"
#include "RifEclipseOutputFileTools.h"
#include "RifEclipseUnifiedRestartFileAccess.h"
#include "RifEclipseRestartFilesetAccess.h"
#include "RifReaderInterface.h"

#include <iostream>

#include "ecl_grid.h"
#include "well_state.h"
#include "ecl_kw_magic.h"

#include "cafProgressInfo.h"

//--------------------------------------------------------------------------------------------------
///     ECLIPSE cell numbering layout:
///        Lower layer:   Upper layer
///
///       2---3           6---7
///       |   |           |   |
///       0---1           4---5
///
///
///
//--------------------------------------------------------------------------------------------------

// The indexing conventions for vertices in ECLIPSE
//
//      6-------------7
//     /|            /|
//    / |           / |
//   /  |          /  |
//  4-------------5   |
//  |   |         |   |
//  |   2---------|---3
//  |  /          |  /
//  | /           | /
//  |/            |/
//  0-------------1
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

bool transferGridCellData(RigMainGrid* mainGrid, RigActiveCellInfo* activeCellInfo, RigGridBase* localGrid, const ecl_grid_type* localEclGrid, size_t matrixActiveStartIndex, size_t fractureActiveStartIndex)
{
    CVF_ASSERT(activeCellInfo);

    int cellCount = ecl_grid_get_global_size(localEclGrid);
    size_t cellStartIndex = mainGrid->cells().size();
    size_t nodeStartIndex = mainGrid->nodes().size();

    RigCell defaultCell;
    defaultCell.setHostGrid(localGrid);
    mainGrid->cells().resize(cellStartIndex + cellCount, defaultCell);

    mainGrid->nodes().resize(nodeStartIndex + cellCount*8, cvf::Vec3d(0,0,0));

    int progTicks = 100;
    double cellsPrProgressTick = cellCount/(float)progTicks;
    caf::ProgressInfo progInfo(progTicks, "");
    size_t computedCellCount = 0;
    // Loop over cells and fill them with data

#pragma omp parallel for
    for (int localCellIdx = 0; localCellIdx < cellCount; ++localCellIdx)
    {
        RigCell& cell = mainGrid->cells()[cellStartIndex + localCellIdx];

        bool invalid = ecl_grid_cell_invalid1(localEclGrid, localCellIdx);
        cell.setInvalid(invalid);
        cell.setCellIndex(localCellIdx);

        // Active cell index

        int matrixActiveIndex = ecl_grid_get_active_index1(localEclGrid, localCellIdx);
        if (matrixActiveIndex != -1)
        {
            activeCellInfo->setActiveIndexInMatrixModel(cellStartIndex + localCellIdx, matrixActiveStartIndex + matrixActiveIndex);
        }

        int fractureActiveIndex = ecl_grid_get_active_fracture_index1(localEclGrid, localCellIdx);
        if (fractureActiveIndex != -1)
        {
            activeCellInfo->setActiveIndexInFractureModel(cellStartIndex + localCellIdx, fractureActiveStartIndex + fractureActiveIndex);
        }

        // Parent cell index

        int parentCellIndex = ecl_grid_get_parent_cell1(localEclGrid, localCellIdx);
        if (parentCellIndex == -1)
        {
            cell.setParentCellIndex(cvf::UNDEFINED_SIZE_T);
        }
        else
        {
            cell.setParentCellIndex(parentCellIndex);
        }
    
        // Coarse cell info
        ecl_coarse_cell_type * coarseCellData = ecl_grid_get_cell_coarse_group1( localEclGrid , localCellIdx);
        cell.setInCoarseCell(coarseCellData != NULL);

        // Corner coordinates
        int cIdx;
        for (cIdx = 0; cIdx < 8; ++cIdx)
        {
            double * point = mainGrid->nodes()[nodeStartIndex + localCellIdx * 8 + cellMappingECLRi[cIdx]].ptr();
            ecl_grid_get_corner_xyz1(localEclGrid, localCellIdx, cIdx, &(point[0]), &(point[1]), &(point[2]));
            point[2] = -point[2];
            cell.cornerIndices()[cIdx] = nodeStartIndex + localCellIdx*8 + cIdx;
        }

        // Sub grid in cell
        const ecl_grid_type* subGrid = ecl_grid_get_cell_lgr1(localEclGrid, localCellIdx);
        if (subGrid != NULL)
        {
            int subGridFileIndex = ecl_grid_get_grid_nr(subGrid);
            CVF_ASSERT(subGridFileIndex > 0);
            cell.setSubGrid(static_cast<RigLocalGrid*>(mainGrid->gridByIndex(subGridFileIndex)));
        }

        // Mark inactive long pyramid looking cells as invalid
        // Forslag
        //if (!invalid && (cell.isInCoarseCell() || (!cell.isActiveInMatrixModel() && !cell.isActiveInFractureModel()) ) )
        if (!invalid)
        {
            cell.setInvalid(cell.isLongPyramidCell());
        }

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
    m_fileSet.clear();

    m_timeSteps.clear();

    m_eclipseCase = NULL;

    m_ecl_init_file = NULL;
    m_dynamicResultsAccess = NULL;
}

//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
RifReaderEclipseOutput::~RifReaderEclipseOutput()
{
}


//--------------------------------------------------------------------------------------------------
/// Close interface (for now, no files are kept open after calling methods, so just clear members)
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::close()
{
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
/// Read geometry from file given by name into given reservoir object
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::transferGeometry(const ecl_grid_type* mainEclGrid, RigEclipseCase* eclipseCase)
{
    CVF_ASSERT(eclipseCase);

    if (!mainEclGrid)
    {
        // Some error
        return false;
    }

    RigActiveCellInfo* activeCellInfo = eclipseCase->activeCellInfo();
    CVF_ASSERT(activeCellInfo);

    RigMainGrid* mainGrid = eclipseCase->mainGrid();
    {
        cvf::Vec3st  gridPointDim(0,0,0);
        gridPointDim.x() = ecl_grid_get_nx(mainEclGrid) + 1;
        gridPointDim.y() = ecl_grid_get_ny(mainEclGrid) + 1;
        gridPointDim.z() = ecl_grid_get_nz(mainEclGrid) + 1;
        mainGrid->setGridPointDimensions(gridPointDim);
    }

    // Get and set grid and lgr metadata

    size_t totalCellCount = static_cast<size_t>(ecl_grid_get_global_size(mainEclGrid));

    int numLGRs = ecl_grid_get_num_lgr(mainEclGrid);
    int lgrIdx;
    for (lgrIdx = 0; lgrIdx < numLGRs; ++lgrIdx)
    {
        ecl_grid_type* localEclGrid = ecl_grid_iget_lgr(mainEclGrid, lgrIdx);

        std::string lgrName = ecl_grid_get_name(localEclGrid);
        cvf::Vec3st  gridPointDim(0,0,0);
        gridPointDim.x() = ecl_grid_get_nx(localEclGrid) + 1;
        gridPointDim.y() = ecl_grid_get_ny(localEclGrid) + 1;
        gridPointDim.z() = ecl_grid_get_nz(localEclGrid) + 1;

        RigLocalGrid* localGrid = new RigLocalGrid(mainGrid);
        mainGrid->addLocalGrid(localGrid);

        localGrid->setIndexToStartOfCells(totalCellCount);
        localGrid->setGridName(lgrName);
        localGrid->setGridPointDimensions(gridPointDim);

        totalCellCount += ecl_grid_get_global_size(localEclGrid);
    }
    
    activeCellInfo->setGlobalCellCount(totalCellCount);

    // Reserve room for the cells and nodes and fill them with data

    mainGrid->cells().reserve(totalCellCount);
    mainGrid->nodes().reserve(8*totalCellCount);

    caf::ProgressInfo progInfo(3 + numLGRs, "");
    progInfo.setProgressDescription("Main Grid");
    progInfo.setNextProgressIncrement(3);

    transferGridCellData(mainGrid, activeCellInfo, mainGrid, mainEclGrid, 0, 0);

    progInfo.setProgress(3);

    size_t globalMatrixActiveSize = ecl_grid_get_nactive(mainEclGrid);
    size_t globalFractureActiveSize = ecl_grid_get_nactive_fracture(mainEclGrid);

    activeCellInfo->setGridCount(1 + numLGRs);
    activeCellInfo->setGridActiveCellCounts(0, globalMatrixActiveSize, globalFractureActiveSize);

    mainGrid->setMatrixModelActiveCellCount(globalMatrixActiveSize);
    mainGrid->setFractureModelActiveCellCount(globalFractureActiveSize);

    for (lgrIdx = 0; lgrIdx < numLGRs; ++lgrIdx)
    {
        progInfo.setProgressDescription("LGR number " + QString::number(lgrIdx+1));

        ecl_grid_type* localEclGrid = ecl_grid_iget_lgr(mainEclGrid, lgrIdx);
        RigLocalGrid* localGrid = static_cast<RigLocalGrid*>(mainGrid->gridByIndex(lgrIdx+1));

        transferGridCellData(mainGrid, activeCellInfo, localGrid, localEclGrid, globalMatrixActiveSize, globalFractureActiveSize);

        int matrixActiveCellCount = ecl_grid_get_nactive(localEclGrid);
        localGrid->setMatrixModelActiveCellCount(matrixActiveCellCount);
        globalMatrixActiveSize += matrixActiveCellCount;

        int fractureActiveCellCount = ecl_grid_get_nactive_fracture(localEclGrid);
        localGrid->setFractureModelActiveCellCount(fractureActiveCellCount);
        globalFractureActiveSize += fractureActiveCellCount;

        activeCellInfo->setGridActiveCellCounts(lgrIdx + 1, matrixActiveCellCount, fractureActiveCellCount);

        progInfo.setProgress(3 + lgrIdx);
    }

    activeCellInfo->computeDerivedData();

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Open file and read geometry into given reservoir object
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::open(const QString& fileName, RigEclipseCase* eclipseCase)
{
    CVF_ASSERT(eclipseCase);
    caf::ProgressInfo progInfo(100, "");

    progInfo.setProgressDescription("Reading Grid");

    // Make sure everything's closed
    close();

    // Get set of files
    QStringList fileSet;
    if (!RifEclipseOutputFileTools::fileSet(fileName, &fileSet)) return false;
    
    progInfo.incrementProgress();

    progInfo.setNextProgressIncrement(20);
    // Keep the set of files of interest
    m_fileSet = fileSet;

    // Read geometry
    ecl_grid_type * mainEclGrid = ecl_grid_alloc( fileName.toAscii().data() );

    progInfo.incrementProgress();

    progInfo.setNextProgressIncrement(10);
    progInfo.setProgressDescription("Transferring grid geometry");

    if (!transferGeometry(mainEclGrid, eclipseCase)) return false;
    progInfo.incrementProgress();

    progInfo.setProgressDescription("Releasing reader memory");
    ecl_grid_free( mainEclGrid );
    progInfo.incrementProgress();

    progInfo.setProgressDescription("Reading Result index");
    progInfo.setNextProgressIncrement(60);

    m_eclipseCase = eclipseCase;
    
    eclipseCase->results(RifReaderInterface::MATRIX_RESULTS)->setReaderInterface(this);
    eclipseCase->results(RifReaderInterface::FRACTURE_RESULTS)->setReaderInterface(this);
    
    // Build results meta data
    if (!buildMetaData()) return false;
    progInfo.incrementProgress();

    progInfo.setNextProgressIncrement(8);
    progInfo.setProgressDescription("Reading Well information");
    
    readWellCells();


    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::openAndReadActiveCellData(const QString& fileName, RigEclipseCase* mainEclipseCase, RigEclipseCase* eclipseCase)
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
    if (!RifEclipseOutputFileTools::fileSet(fileName, &fileSet)) return false;

    // Keep the set of files of interest
    m_fileSet = fileSet;
    m_eclipseCase = eclipseCase;

    eclipseCase->results(RifReaderInterface::MATRIX_RESULTS)->setReaderInterface(this);
    eclipseCase->results(RifReaderInterface::FRACTURE_RESULTS)->setReaderInterface(this);

    if (!readActiveCellInfo())
    {
        return false;
    }
    

    // Reading of metadata and well cells is not performed here
    //if (!buildMetaData()) return false;
    // readWellCells();

    m_dynamicResultsAccess = createDynamicResultsAccess(m_fileSet);

    QList<QDateTime> mainCaseTimeSteps = mainEclipseCase->results(RifReaderInterface::MATRIX_RESULTS)->readerInterface()->timeSteps();
    m_dynamicResultsAccess->setTimeSteps(mainCaseTimeSteps);

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::readActiveCellInfo()
{
    CVF_ASSERT(m_eclipseCase.notNull());
    CVF_ASSERT(m_eclipseCase->mainGrid());

    QString egridFileName = RifEclipseOutputFileTools::fileNameByType(m_fileSet, ECL_EGRID_FILE);
    if (egridFileName.size() > 0)
    {
        ecl_file_type* ecl_file = ecl_file_open(egridFileName.toAscii().data());
        if (!ecl_file) return false;

        int actnumKeywordCount = ecl_file_get_num_named_kw(ecl_file, ACTNUM_KW);
        if (actnumKeywordCount > 0)
        {
            std::vector<std::vector<int> > actnumValuesPerGrid;
            actnumValuesPerGrid.resize(actnumKeywordCount);

            size_t globalCellCount = 0;
            for (size_t gridIdx = 0; gridIdx < static_cast<size_t>(actnumKeywordCount); gridIdx++)
            {
                RifEclipseOutputFileTools::keywordData(ecl_file, ACTNUM_KW, gridIdx, &actnumValuesPerGrid[gridIdx]);

                globalCellCount += actnumValuesPerGrid[gridIdx].size();
            }

            // Check if number of cells is matching
            if (m_eclipseCase->mainGrid()->cells().size() != globalCellCount)
            {
                return false;
            }

            RigActiveCellInfo* activeCellInfo = m_eclipseCase->activeCellInfo();

            activeCellInfo->setGlobalCellCount(globalCellCount);
            activeCellInfo->setGridCount(actnumKeywordCount);

            size_t cellIdx = 0;
            for (size_t gridIdx = 0; gridIdx < static_cast<size_t>(actnumKeywordCount); gridIdx++)
            {
                size_t activeMatrixIndex = 0;
                size_t activeFractureIndex = 0;

                std::vector<int>& actnumValues = actnumValuesPerGrid[gridIdx];

                for (size_t i = 0; i < actnumValues.size(); i++)
                {
                    if (actnumValues[i] == 1 || actnumValues[i] == 3)
                    {
                        activeCellInfo->setActiveIndexInMatrixModel(cellIdx, activeMatrixIndex++);
                    }

                    if (actnumValues[i] == 2 || actnumValues[i] == 3)
                    {
                        activeCellInfo->setActiveIndexInFractureModel(cellIdx, activeFractureIndex++);
                    }

                    cellIdx++;
                }

                activeCellInfo->setGridActiveCellCounts(gridIdx, activeMatrixIndex, activeFractureIndex);
            }

            activeCellInfo->computeDerivedData();
        }

        ecl_file_close(ecl_file);

        return true;
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/// Build meta data - get states and results info
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::buildMetaData()
{
    CVF_ASSERT(m_eclipseCase.notNull());
    CVF_ASSERT(m_fileSet.size() > 0);

    caf::ProgressInfo progInfo(m_fileSet.size() + 3,"");

    progInfo.setNextProgressIncrement(m_fileSet.size());

    // Create access object for dynamic results
    m_dynamicResultsAccess = createDynamicResultsAccess(m_fileSet);
    if (m_dynamicResultsAccess.isNull())
    {
        return false;
    }

    m_dynamicResultsAccess->open(m_fileSet);


    progInfo.incrementProgress();

    RigReservoirCellResults* matrixModelResults = m_eclipseCase->results(RifReaderInterface::MATRIX_RESULTS);
    RigReservoirCellResults* fractureModelResults = m_eclipseCase->results(RifReaderInterface::FRACTURE_RESULTS);

    if (m_dynamicResultsAccess.notNull())
    {
        // Get time steps 
        m_timeSteps = m_dynamicResultsAccess->timeSteps();

        QStringList resultNames;
        std::vector<size_t> resultNamesDataItemCounts;
        m_dynamicResultsAccess->resultNames(&resultNames, &resultNamesDataItemCounts);

        {
            QStringList matrixResultNames = validKeywordsForPorosityModel(resultNames, resultNamesDataItemCounts, m_eclipseCase->activeCellInfo(), RifReaderInterface::MATRIX_RESULTS, m_dynamicResultsAccess->timeStepCount());

            for (int i = 0; i < matrixResultNames.size(); ++i)
            {
                size_t resIndex = matrixModelResults->addEmptyScalarResult(RimDefines::DYNAMIC_NATIVE, matrixResultNames[i]);
                matrixModelResults->setTimeStepDates(resIndex, m_timeSteps);
            }
        }

        {
            QStringList fractureResultNames = validKeywordsForPorosityModel(resultNames, resultNamesDataItemCounts, m_eclipseCase->activeCellInfo(), RifReaderInterface::FRACTURE_RESULTS, m_dynamicResultsAccess->timeStepCount());

            for (int i = 0; i < fractureResultNames.size(); ++i)
            {
                size_t resIndex = fractureModelResults->addEmptyScalarResult(RimDefines::DYNAMIC_NATIVE, fractureResultNames[i]);
                fractureModelResults->setTimeStepDates(resIndex, m_timeSteps);
            }
        }

    }

    progInfo.incrementProgress();

    openInitFile();
    
    progInfo.incrementProgress();

    if (m_ecl_init_file)
    {
        QStringList resultNames;
        std::vector<size_t> resultNamesDataItemCounts;
        RifEclipseOutputFileTools::findKeywordsAndDataItemCounts(m_ecl_init_file, &resultNames, &resultNamesDataItemCounts);

        {
            QStringList matrixResultNames = validKeywordsForPorosityModel(resultNames, resultNamesDataItemCounts, m_eclipseCase->activeCellInfo(), RifReaderInterface::MATRIX_RESULTS, 1);

            QList<QDateTime> staticDate;
            if (m_timeSteps.size() > 0)
            {
                staticDate.push_back(m_timeSteps.front());
            }

            for (int i = 0; i < matrixResultNames.size(); ++i)
            {
                size_t resIndex = matrixModelResults->addEmptyScalarResult(RimDefines::STATIC_NATIVE, matrixResultNames[i]);
                matrixModelResults->setTimeStepDates(resIndex, staticDate);
            }
        }

        {
            QStringList fractureResultNames = validKeywordsForPorosityModel(resultNames, resultNamesDataItemCounts, m_eclipseCase->activeCellInfo(), RifReaderInterface::FRACTURE_RESULTS, 1);

            QList<QDateTime> staticDate;
            if (m_timeSteps.size() > 0)
            {
                staticDate.push_back(m_timeSteps.front());
            }

            for (int i = 0; i < fractureResultNames.size(); ++i)
            {
                size_t resIndex = fractureModelResults->addEmptyScalarResult(RimDefines::STATIC_NATIVE, fractureResultNames[i]);
                fractureModelResults->setTimeStepDates(resIndex, staticDate);
            }
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Create results access object (.UNRST or .X0001 ... .XNNNN)
//--------------------------------------------------------------------------------------------------
RifEclipseRestartDataAccess* RifReaderEclipseOutput::createDynamicResultsAccess(const QStringList& fileSet)
{
    RifEclipseRestartDataAccess* resultsAccess = NULL;

    // Look for unified restart file
    QString unrstFileName = RifEclipseOutputFileTools::fileNameByType(fileSet, ECL_UNIFIED_RESTART_FILE);
    if (unrstFileName.size() > 0)
    {
        resultsAccess = new RifEclipseUnifiedRestartFileAccess();
        resultsAccess->setFileSet(QStringList(unrstFileName));
    }
    else
    {
        // Look for set of restart files (one file per time step)
        QStringList restartFiles = RifEclipseOutputFileTools::fileNamesByType(fileSet, ECL_RESTART_FILE);
        if (restartFiles.size() > 0)
        {
            resultsAccess = new RifEclipseRestartFilesetAccess();
            resultsAccess->setFileSet(restartFiles);
        }
    }

    return resultsAccess;
}

//--------------------------------------------------------------------------------------------------
/// Get all values of a given static result as doubles
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::staticResult(const QString& result, PorosityModelResultType matrixOrFracture, std::vector<double>* values)
{
    CVF_ASSERT(values);

    if (!openInitFile())
    {
        return false;
    }

    CVF_ASSERT(m_ecl_init_file);

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

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Get dynamic result at given step index. Will concatenate values for the main grid and all sub grids.
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::dynamicResult(const QString& result, PorosityModelResultType matrixOrFracture, size_t stepIndex, std::vector<double>* values)
{
    if (m_dynamicResultsAccess.isNull())
    {
        m_dynamicResultsAccess = createDynamicResultsAccess(m_fileSet);
    }

    if (m_dynamicResultsAccess.isNull())
    {
        CVF_ASSERT(false);
        return false;
    }

    std::vector<double> fileValues;
    if (!m_dynamicResultsAccess->results(result, stepIndex, m_eclipseCase->mainGrid()->gridCount(), &fileValues))
    {
        return false;
    }

    extractResultValuesBasedOnPorosityModel(matrixOrFracture, values, fileValues);

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::readWellCells()
{
    CVF_ASSERT(m_eclipseCase.notNull());

    if (m_dynamicResultsAccess.isNull()) return;

    well_info_type* ert_well_info = well_info_alloc(NULL);
    if (!ert_well_info) return;

    m_dynamicResultsAccess->readWellData(ert_well_info);

    RigMainGrid* mainGrid = m_eclipseCase->mainGrid();
    std::vector<RigGridBase*> grids;
    m_eclipseCase->allGrids(&grids);

    cvf::Collection<RigWellResults> wells;
    caf::ProgressInfo progress(well_info_get_num_wells(ert_well_info), "");

    int wellIdx;
    for (wellIdx = 0; wellIdx < well_info_get_num_wells(ert_well_info); wellIdx++)
    {
        const char* wellName = well_info_iget_well_name(ert_well_info, wellIdx);
        CVF_ASSERT(wellName);

        cvf::ref<RigWellResults> wellResults = new RigWellResults;
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
            // Also see RifEclipseOutputFileAccess::timeStepsText for accessing time_t structures
            {
                time_t stepTime = well_state_get_sim_time(ert_well_state);
                wellResFrame.m_timestamp = QDateTime::fromTime_t(stepTime);
            }

            // Production type
            well_type_enum ert_well_type = well_state_get_type(ert_well_state);
            if (ert_well_type == PRODUCER)
            {
                wellResFrame.m_productionType = RigWellResultFrame::PRODUCER;
            }
            else if (ert_well_type == WATER_INJECTOR)
            {
                wellResFrame.m_productionType = RigWellResultFrame::WATER_INJECTOR;
            }
            else if (ert_well_type == GAS_INJECTOR)
            {
                wellResFrame.m_productionType = RigWellResultFrame::GAS_INJECTOR;
            }
            else if (ert_well_type == OIL_INJECTOR)
            {
                wellResFrame.m_productionType = RigWellResultFrame::OIL_INJECTOR;
            }
            else
            {
                wellResFrame.m_productionType = RigWellResultFrame::UNDEFINED_PRODUCTION_TYPE;
            }

            wellResFrame.m_isOpen = well_state_is_open( ert_well_state );




            // Loop over all the grids in the model. If we have connections in one, we will discard
            // the maingrid connections as they are "duplicates"

            bool hasWellConnectionsInLGR = false;
            for (size_t gridNr = 1; gridNr < grids.size(); ++gridNr)
            {
                int branchCount = well_state_iget_lgr_num_branches(ert_well_state, static_cast<int>(gridNr));
                if (branchCount > 0)
                {
                    hasWellConnectionsInLGR = true;
                    break;
                }
            }
            size_t gridNr = hasWellConnectionsInLGR ? 1 : 0;
            for (; gridNr < grids.size(); ++gridNr)
            {

                // Wellhead. If several grids have a wellhead definition for this well, we use tha last one. (Possibly the innermost LGR)
                const well_conn_type* ert_wellhead = well_state_iget_wellhead(ert_well_state, static_cast<int>(gridNr));
                if (ert_wellhead)
                {
                    int cellI = well_conn_get_i( ert_wellhead );
                    int cellJ = well_conn_get_j( ert_wellhead );
                    int cellK = CVF_MAX(0, well_conn_get_k(ert_wellhead)); // Why this ?

                    // If a well is defined in fracture region, the K-value is from (cellCountK - 1) -> cellCountK*2 - 1
                    // Adjust K so index is always in valid grid region
                    if (cellK >= static_cast<int>(grids[gridNr]->cellCountK()))
                    {
                        cellK -= static_cast<int>(grids[gridNr]->cellCountK());
                    }
                    
                    wellResFrame.m_wellHead.m_gridCellIndex = grids[gridNr]->cellIndexFromIJK(cellI, cellJ, cellK);
                    wellResFrame.m_wellHead.m_gridIndex = gridNr;
                }


                int branchCount = well_state_iget_lgr_num_branches(ert_well_state, static_cast<int>(gridNr));
                if (branchCount > 0)
                {
                    if (static_cast<int>(wellResFrame.m_wellResultBranches.size()) < branchCount) wellResFrame.m_wellResultBranches.resize(branchCount);

                    for (int branchIdx = 0; branchIdx < branchCount; ++branchIdx )
                    {
                        // Connections
                        int connectionCount = well_state_iget_num_lgr_connections(ert_well_state, static_cast<int>(gridNr), branchIdx);
                        if (connectionCount > 0)
                        {

                            RigWellResultBranch& wellSegment = wellResFrame.m_wellResultBranches[branchIdx]; // Is this completely right? Is the branch index actually the same between lgrs ?
                            wellSegment.m_branchNumber = branchIdx;
                            size_t existingConnCount = wellSegment.m_wellCells.size();
                            wellSegment.m_wellCells.resize(existingConnCount + connectionCount);

                            int connIdx;
                            for (connIdx = 0; connIdx < connectionCount; connIdx++)
                            {
                                const well_conn_type* ert_connection = well_state_iget_lgr_connections( ert_well_state, static_cast<int>(gridNr), branchIdx)[connIdx];
                                CVF_ASSERT(ert_connection);

                                RigWellResultCell& data = wellSegment.m_wellCells[existingConnCount + connIdx];
                                data.m_gridIndex = gridNr;
                                {
                                    int cellI = well_conn_get_i( ert_connection );
                                    int cellJ = well_conn_get_j( ert_connection );
                                    int cellK = well_conn_get_k( ert_connection );
                                    bool open = well_conn_open( ert_connection );
                                    int branch = well_conn_get_branch( ert_connection );
                                    int segment = well_conn_get_segment( ert_connection );
                                    
                                    // If a well is defined in fracture region, the K-value is from (cellCountK - 1) -> cellCountK*2 - 1
                                    // Adjust K so index is always in valid grid region
                                    if (cellK >= static_cast<int>(grids[gridNr]->cellCountK()))
                                    {
                                        cellK -= static_cast<int>(grids[gridNr]->cellCountK());
                                    }

                                    data.m_gridCellIndex = grids[gridNr]->cellIndexFromIJK(cellI , cellJ , cellK);
                                    
                                    data.m_isOpen    = open;
                                    data.m_branchId  = branch;
                                    data.m_segmentId = segment;
                                }
                            }
                        }
                    }
                }
            }
        }

        wellResults->computeMappingFromResultTimeIndicesToWellTimeIndices(m_timeSteps);

        wells.push_back(wellResults.p());

        progress.incrementProgress();
    }

    well_info_free(ert_well_info);

    m_eclipseCase->setWellResults(wells);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RifReaderEclipseOutput::validKeywordsForPorosityModel(const QStringList& keywords, const std::vector<size_t>& keywordDataItemCounts, const RigActiveCellInfo* activeCellInfo, PorosityModelResultType matrixOrFracture, size_t timeStepCount) const
{
    CVF_ASSERT(activeCellInfo);

    if (keywords.size() != static_cast<int>(keywordDataItemCounts.size()))
    {
        return QStringList();
    }

    if (matrixOrFracture == RifReaderInterface::FRACTURE_RESULTS)
    {
        if (activeCellInfo->globalFractureModelActiveCellCount() == 0)
        {
            return QStringList();
        }
    }

    QStringList keywordsWithCorrectNumberOfDataItems;
    
    for (int i = 0; i < keywords.size(); i++)
    {
        QString keyword = keywords[i];
        size_t keywordDataCount = keywordDataItemCounts[i];

        if (activeCellInfo->globalMatrixModelActiveCellCount() > 0)
        {
            size_t timeStepsMatrix = keywordDataItemCounts[i] / activeCellInfo->globalMatrixModelActiveCellCount();
            size_t timeStepsMatrixRest = keywordDataItemCounts[i] % activeCellInfo->globalMatrixModelActiveCellCount();
        
            size_t timeStepsMatrixAndFracture = keywordDataItemCounts[i] / (activeCellInfo->globalMatrixModelActiveCellCount() + activeCellInfo->globalFractureModelActiveCellCount());
            size_t timeStepsMatrixAndFractureRest = keywordDataItemCounts[i] % (activeCellInfo->globalMatrixModelActiveCellCount() + activeCellInfo->globalFractureModelActiveCellCount());

            if (matrixOrFracture == RifReaderInterface::MATRIX_RESULTS)
            {
                if (timeStepsMatrixRest == 0 || timeStepsMatrixAndFractureRest == 0)
                {
                    if (timeStepCount == timeStepsMatrix || timeStepCount == timeStepsMatrixAndFracture)
                    {
                        keywordsWithCorrectNumberOfDataItems.push_back(keywords[i]);
                    }
                }
            }
            else
            {
                if (timeStepsMatrixAndFractureRest == 0 && timeStepCount == timeStepsMatrixAndFracture)
                {
                    keywordsWithCorrectNumberOfDataItems.push_back(keywords[i]);
                }
            }
        }
        else
        {
            keywordsWithCorrectNumberOfDataItems.push_back(keywords[i]);
        }
    }

    return keywordsWithCorrectNumberOfDataItems;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::extractResultValuesBasedOnPorosityModel(PorosityModelResultType matrixOrFracture, std::vector<double>* destinationResultValues, const std::vector<double>& sourceResultValues)
{
    RigActiveCellInfo* actCellInfo = m_eclipseCase->activeCellInfo();

    if (matrixOrFracture == RifReaderInterface::MATRIX_RESULTS)
    {
        if (actCellInfo->globalFractureModelActiveCellCount() == 0)
        {
            destinationResultValues->insert(destinationResultValues->end(), sourceResultValues.begin(), sourceResultValues.end());
        }
        else
        {
            size_t dataItemCount = 0;
            size_t sourceStartPosition = 0;

            for (size_t i = 0; i < m_eclipseCase->mainGrid()->gridCount(); i++)
            {
                size_t matrixActiveCellCount = 0;
                size_t fractureActiveCellCount = 0;
                actCellInfo->gridActiveCellCounts(i, matrixActiveCellCount, fractureActiveCellCount);

                destinationResultValues->insert(destinationResultValues->end(), sourceResultValues.begin() + sourceStartPosition, sourceResultValues.begin() + sourceStartPosition + matrixActiveCellCount);

                sourceStartPosition += (matrixActiveCellCount + fractureActiveCellCount);
            }
        }
    }
    else
    {
        size_t dataItemCount = 0;
        size_t sourceStartPosition = 0;

        for (size_t i = 0; i < m_eclipseCase->mainGrid()->gridCount(); i++)
        {
            size_t matrixActiveCellCount = 0;
            size_t fractureActiveCellCount = 0;
            actCellInfo->gridActiveCellCounts(i, matrixActiveCellCount, fractureActiveCellCount);

            destinationResultValues->insert(destinationResultValues->end(), sourceResultValues.begin() + sourceStartPosition + matrixActiveCellCount, sourceResultValues.begin() + sourceStartPosition + matrixActiveCellCount + fractureActiveCellCount);

            sourceStartPosition += (matrixActiveCellCount + fractureActiveCellCount);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::openInitFile()
{
    if (m_ecl_init_file)
    {
        return true;
    }

    QString initFileName = RifEclipseOutputFileTools::fileNameByType(m_fileSet, ECL_INIT_FILE);
    if (initFileName.size() > 0)
    {
        m_ecl_init_file = ecl_file_open(initFileName.toAscii().data());
        if (m_ecl_init_file)
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<QDateTime> RifReaderEclipseOutput::timeSteps()
{
    return m_timeSteps;
}

