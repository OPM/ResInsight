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
#include "RigReservoir.h"
#include "RigReservoirCellResults.h"

#include "RifReaderEclipseOutput.h"
#include "RifEclipseOutputFileTools.h"
#include "RifEclipseUnifiedRestartFileAccess.h"
#include "RifEclipseRestartFilesetAccess.h"
#include "RifReaderInterface.h"

#include <iostream>

#include "ecl_grid.h"
#include "well_state.h"
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

bool transferGridCellData(RigMainGrid* mainGrid, RigGridBase* localGrid, const ecl_grid_type* localEclGrid, size_t matrixActiveStartIndex, size_t fractureActiveStartIndex)
{
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
    for (int gIdx = 0; gIdx < cellCount; ++gIdx)
    {
        RigCell& cell = mainGrid->cells()[cellStartIndex + gIdx];

        bool invalid = ecl_grid_cell_invalid1(localEclGrid, gIdx);
        cell.setInvalid(invalid);
        cell.setCellIndex(gIdx);

        // Active cell index

        int matrixActiveIndex = ecl_grid_get_active_index1(localEclGrid, gIdx);
        if (matrixActiveIndex != -1)
        {
            cell.setActiveIndexInMatrixModel(matrixActiveStartIndex + matrixActiveIndex);
        }
        else
        {
            cell.setActiveIndexInMatrixModel(cvf::UNDEFINED_SIZE_T);
        }

        int fractureActiveIndex = ecl_grid_get_active_fracture_index1(localEclGrid, gIdx);
        if (fractureActiveIndex != -1)
        {
            cell.setActiveIndexInFractureModel(fractureActiveStartIndex + fractureActiveIndex);
        }
        else
        {
            cell.setActiveIndexInFractureModel(cvf::UNDEFINED_SIZE_T);
        }

        // Parent cell index

        int parentCellIndex = ecl_grid_get_parent_cell1(localEclGrid, gIdx);
        if (parentCellIndex == -1)
        {
            cell.setParentCellIndex(cvf::UNDEFINED_SIZE_T);
        }
        else
        {
            cell.setParentCellIndex(parentCellIndex);
        }
    
        // Coarse cell info
        ecl_coarse_cell_type * coarseCellData = ecl_grid_get_cell_coarse_group1( localEclGrid , gIdx);
        cell.setInCoarseCell(coarseCellData != NULL);

        // Corner coordinates
        int cIdx;
        for (cIdx = 0; cIdx < 8; ++cIdx)
        {
            double * point = mainGrid->nodes()[nodeStartIndex + gIdx * 8 + cellMappingECLRi[cIdx]].ptr();
            ecl_grid_get_corner_xyz1(localEclGrid, gIdx, cIdx, &(point[0]), &(point[1]), &(point[2]));
            point[2] = -point[2];
            cell.cornerIndices()[cIdx] = nodeStartIndex + gIdx*8 + cIdx;
        }

        // Sub grid in cell
        const ecl_grid_type* subGrid = ecl_grid_get_cell_lgr1(localEclGrid, gIdx);
        if (subGrid != NULL)
        {
            int subGridFileIndex = ecl_grid_get_grid_nr(subGrid);
            CVF_ASSERT(subGridFileIndex > 0);
            cell.setSubGrid(static_cast<RigLocalGrid*>(mainGrid->gridByIndex(subGridFileIndex)));
        }

        // Mark inactive long pyramid looking cells as invalid
        if (!invalid && (cell.isInCoarseCell() || (!cell.isActiveInMatrixModel() && !cell.isActiveInFractureModel()) ) )
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
    ground();
}

//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
RifReaderEclipseOutput::~RifReaderEclipseOutput()
{
}

//--------------------------------------------------------------------------------------------------
/// Ground members
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::ground()
{
    m_fileName.clear();
    m_fileSet.clear();

    m_timeSteps.clear();

}

//--------------------------------------------------------------------------------------------------
/// Close interface (for now, no files are kept open after calling methods, so just clear members)
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::close()
{
    m_staticResultsAccess     = NULL;
    m_dynamicResultsAccess    = NULL;

    ground();
}

//--------------------------------------------------------------------------------------------------
/// Read geometry from file given by name into given reservoir object
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::transferGeometry(const ecl_grid_type* mainEclGrid, RigReservoir* reservoir)
{
    CVF_ASSERT(reservoir);

    if (!mainEclGrid)
    {
        // Some error
        return false;
    }

    RigMainGrid* mainGrid = reservoir->mainGrid();
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

    // Reserve room for the cells and nodes and fill them with data

    mainGrid->cells().reserve(totalCellCount);
    mainGrid->nodes().reserve(8*totalCellCount);

    caf::ProgressInfo progInfo(3 + numLGRs, "");
    progInfo.setProgressDescription("Main Grid");
    progInfo.setNextProgressIncrement(3);

    transferGridCellData(mainGrid, mainGrid, mainEclGrid, 0, 0);

    progInfo.setProgress(3);

    size_t globalMatrixActiveSize = ecl_grid_get_nactive(mainEclGrid);
    size_t globalFractureActiveSize = ecl_grid_get_nactive_fracture(mainEclGrid);

    mainGrid->setMatrixModelActiveCellCount(globalMatrixActiveSize);
    mainGrid->setFractureModelActiveCellCount(globalFractureActiveSize);

    for (lgrIdx = 0; lgrIdx < numLGRs; ++lgrIdx)
    {
        progInfo.setProgressDescription("LGR number " + QString::number(lgrIdx+1));

        ecl_grid_type* localEclGrid = ecl_grid_iget_lgr(mainEclGrid, lgrIdx);
        RigLocalGrid* localGrid = static_cast<RigLocalGrid*>(mainGrid->gridByIndex(lgrIdx+1));

        transferGridCellData(mainGrid, localGrid, localEclGrid, globalMatrixActiveSize, globalFractureActiveSize);

        int activeCellCount = ecl_grid_get_nactive(localEclGrid);
        localGrid->setMatrixModelActiveCellCount(activeCellCount);
        globalMatrixActiveSize += activeCellCount;

        activeCellCount = ecl_grid_get_nactive_fracture(localEclGrid);
        localGrid->setFractureModelActiveCellCount(activeCellCount);
        globalFractureActiveSize += activeCellCount;

        progInfo.setProgress(3 + lgrIdx);
    }


    mainGrid->setGlobalMatrixModelActiveCellCount(globalMatrixActiveSize);
    mainGrid->setGlobalFractureModelActiveCellCount(globalFractureActiveSize);

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Open file and read geometry into given reservoir object
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::open(const QString& fileName, RigReservoir* reservoir)
{
    CVF_ASSERT(reservoir);
    caf::ProgressInfo progInfo(10, "");

    progInfo.setProgressDescription("Reading Grid");

    // Make sure everything's closed
    close();

    // Get set of files
    QStringList fileSet;
    if (!RifEclipseOutputFileTools::fileSet(fileName, &fileSet)) return false;


    // Keep the set of files of interest
    m_fileSet = fileSet;

    // Read geometry
    ecl_grid_type * mainEclGrid = ecl_grid_alloc( fileName.toAscii().data() );

    progInfo.setProgress(1);
    progInfo.setNextProgressIncrement(6);
    progInfo.setProgressDescription("Transferring grid geometry");

    if (!transferGeometry(mainEclGrid, reservoir)) return false;
    progInfo.setProgress(7);
    progInfo.setProgressDescription("Releasing reader memory");
    ecl_grid_free( mainEclGrid );

    progInfo.setProgress(8);
    progInfo.setProgressDescription("Reading Result index");

    // Build results meta data
    if (!buildMetaData(reservoir)) return false;

    progInfo.setProgress(9);
    progInfo.setProgressDescription("Reading Well information");
    
    readWellCells(reservoir);
    
    return true;
}

//--------------------------------------------------------------------------------------------------
/// Build meta data - get states and results info
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::buildMetaData(RigReservoir* reservoir)
{
    CVF_ASSERT(reservoir);
    CVF_ASSERT(m_fileSet.size() > 0);

    caf::ProgressInfo progInfo(2,"");

    std::vector<size_t> matrixModelActiveCellCounts;
    std::vector<size_t> fractureModelActiveCellCount;

    std::vector<RigGridBase*> grids;
    reservoir->allGrids(&grids);
    for (size_t i = 0; i < grids.size(); i++)
    {
        matrixModelActiveCellCounts.push_back(grids[i]->matrixModelActiveCellCount());
        fractureModelActiveCellCount.push_back(grids[i]->fractureModelActiveCellCount());
    }

    // Create access object for dynamic results
    m_dynamicResultsAccess = dynamicResultsAccess(m_fileSet, matrixModelActiveCellCounts, fractureModelActiveCellCount);
    if (m_dynamicResultsAccess.isNull())
    {
        return false;
    }

    RigReservoirCellResults* matrixModelResults = reservoir->mainGrid()->results(RifReaderInterface::MATRIX_RESULTS);
    RigReservoirCellResults* fractureModelResults = reservoir->mainGrid()->results(RifReaderInterface::FRACTURE_RESULTS);

    if (m_dynamicResultsAccess.notNull())
    {
        // Get time steps 
        m_timeSteps = m_dynamicResultsAccess->timeSteps();

        {
            QStringList dynamicResultNames = m_dynamicResultsAccess->resultNames(RifReaderInterface::MATRIX_RESULTS);

            for (int i = 0; i < dynamicResultNames.size(); ++i)
            {
                size_t resIndex = matrixModelResults->addEmptyScalarResult(RimDefines::DYNAMIC_NATIVE, dynamicResultNames[i]);
                matrixModelResults->setTimeStepDates(resIndex, m_timeSteps);
            }
        }

        {
            QStringList dynamicResultNames = m_dynamicResultsAccess->resultNames(RifReaderInterface::FRACTURE_RESULTS);

            for (int i = 0; i < dynamicResultNames.size(); ++i)
            {
                size_t resIndex = fractureModelResults->addEmptyScalarResult(RimDefines::DYNAMIC_NATIVE, dynamicResultNames[i]);
                fractureModelResults->setTimeStepDates(resIndex, m_timeSteps);
            }
        }

    }

    progInfo.setProgress(1);

    QString initFileName = RifEclipseOutputFileTools::fileNameByType(m_fileSet, ECL_INIT_FILE);
    if (initFileName.size() > 0)
    {
        // Open init file
        cvf::ref<RifEclipseOutputFileTools> initFile = new RifEclipseOutputFileTools;
        if (!initFile->open(initFileName, matrixModelActiveCellCounts, fractureModelActiveCellCount))
        {
            return false;
        }

        {
            QStringList staticResults;
            initFile->validKeywords(&staticResults, RifReaderInterface::MATRIX_RESULTS);
            QStringList staticResultNames = staticResults;

            QList<QDateTime> staticDate;
            if (m_timeSteps.size() > 0)
            {
                staticDate.push_back(m_timeSteps.front());
            }

            for (int i = 0; i < staticResultNames.size(); ++i)
            {
                size_t resIndex = matrixModelResults->addEmptyScalarResult(RimDefines::STATIC_NATIVE, staticResultNames[i]);
                matrixModelResults->setTimeStepDates(resIndex, staticDate);
            }
        }

        {
            QStringList staticResults;
            initFile->validKeywords(&staticResults, RifReaderInterface::FRACTURE_RESULTS);
            QStringList staticResultNames = staticResults;

            QList<QDateTime> staticDate;
            if (m_timeSteps.size() > 0)
            {
                staticDate.push_back(m_timeSteps.front());
            }

            for (int i = 0; i < staticResultNames.size(); ++i)
            {
                size_t resIndex = fractureModelResults->addEmptyScalarResult(RimDefines::STATIC_NATIVE, staticResultNames[i]);
                fractureModelResults->setTimeStepDates(resIndex, staticDate);
            }
        }

        m_staticResultsAccess = initFile;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Create results access object (.UNRST or .X0001 ... .XNNNN)
//--------------------------------------------------------------------------------------------------
RifEclipseRestartDataAccess* RifReaderEclipseOutput::dynamicResultsAccess(const QStringList& fileSet, const std::vector<size_t>& matrixActiveCellCounts, const std::vector<size_t>& fractureActiveCellCounts)
{
    RifEclipseRestartDataAccess* resultsAccess = NULL;

    // Look for unified restart file
    QString unrstFileName = RifEclipseOutputFileTools::fileNameByType(fileSet, ECL_UNIFIED_RESTART_FILE);
    if (unrstFileName.size() > 0)
    {
        resultsAccess = new RifEclipseUnifiedRestartFileAccess();
        if (!resultsAccess->open(QStringList(unrstFileName), matrixActiveCellCounts, fractureActiveCellCounts))
        {
            delete resultsAccess;
            return NULL;
        }
    }
    else
    {
        // Look for set of restart files (one file per time step)
        QStringList restartFiles = RifEclipseOutputFileTools::fileNamesByType(fileSet, ECL_RESTART_FILE);
        if (restartFiles.size() > 0)
        {
            resultsAccess = new RifEclipseRestartFilesetAccess();
            if (!resultsAccess->open(restartFiles, matrixActiveCellCounts, fractureActiveCellCounts))
            {
                delete resultsAccess;
                return NULL;
            }
        }
    }

    // !! could add support for formatted result files
    // !! consider priorities in case multiple types exist (.UNRST, .XNNNN, ...)

    return resultsAccess;
}

//--------------------------------------------------------------------------------------------------
/// Get all values of a given static result as doubles
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::staticResult(const QString& result, PorosityModelResultType matrixOrFracture, std::vector<double>* values)
{
    CVF_ASSERT(values);
    CVF_ASSERT(m_staticResultsAccess.notNull());

    size_t numOccurrences = m_staticResultsAccess->numOccurrences(result);
    size_t i;
    for (i = 0; i < numOccurrences; i++)
    {
        std::vector<double> partValues;
        if (!m_staticResultsAccess->keywordData(result, i, matrixOrFracture, &partValues)) return false;
        values->insert(values->end(), partValues.begin(), partValues.end());
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Get dynamic result at given step index. Will concatenate values for the main grid and all sub grids.
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::dynamicResult(const QString& result, PorosityModelResultType matrixOrFracture, size_t stepIndex, std::vector<double>* values)
{
    CVF_ASSERT(m_dynamicResultsAccess.notNull());
    return m_dynamicResultsAccess->results(result, matrixOrFracture, stepIndex, values);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::readWellCells(RigReservoir* reservoir)
{
    CVF_ASSERT(reservoir);

    if (m_dynamicResultsAccess.isNull()) return;

    well_info_type* ert_well_info = well_info_alloc(NULL);
    if (!ert_well_info) return;

    m_dynamicResultsAccess->readWellData(ert_well_info);

    RigMainGrid* mainGrid = reservoir->mainGrid();
    std::vector<RigGridBase*> grids;
    reservoir->allGrids(&grids);

    cvf::Collection<RigWellResults> wells;

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
                    
                    if (!grids[gridNr]->isCellValid(cellI, cellJ, cellK))
                    {
                        int candidateCellK = findSmallestActiveCellIndexK(grids[gridNr], cellI, cellJ);

                        if (candidateCellK >= 0)
                        {
                            cellK = candidateCellK;
                        }
                        else
                        {
                            cellK = 0;
                        }
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
                                    
                                    if (!grids[gridNr]->isCellValid(cellI, cellJ, cellK))
                                    {
                                        int candidateCellK = findSmallestActiveCellIndexK(grids[gridNr], cellI, cellJ);

                                        if (candidateCellK >= 0)
                                        {
                                            cellK = candidateCellK;
                                        }
                                        else
                                        {
                                            cellK = 0;
                                        }
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
    }

    well_info_free(ert_well_info);

    reservoir->setWellResults(wells);
}

//--------------------------------------------------------------------------------------------------
// For case DUALPORO, the well K index is reported outside the grid. If this happens,
// for the given IJ position, search from K=0 and upwards for first active cell.
//--------------------------------------------------------------------------------------------------
int RifReaderEclipseOutput::findSmallestActiveCellIndexK(const RigGridBase* grid, int cellI, int cellJ)
{
    if (!grid) return -1;

    for (int candidateCellK = 0; candidateCellK < grid->cellCountK(); candidateCellK++ )
    {
        if (grid->isCellActive(cellI, cellJ, candidateCellK))
        {
            return candidateCellK;
        }
    }

    return -1;
}

