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

#include "RifReaderInterfaceEcl.h"
#include "RifReaderEclipseFileAccess.h"
#include "RifReaderEclipseUnifiedRestartFile.h"
#include "RifReaderEclipseRestartFiles.h"


#include <iostream>

#ifdef USE_ECL_LIB
#include "ecl_grid.h"
#include "well_state.h"
#endif //USE_ECL_LIB

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
//      7-------------6
//     /|            /|
//    / |           / |
//   /  |          /  |
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

#ifdef USE_ECL_LIB
bool transferGridCellData(RigMainGrid* mainGrid, RigGridBase* localGrid, ecl_grid_type* localEclGrid, size_t activeStartIndex)
{
    int cellCount = ecl_grid_get_global_size(localEclGrid);
    size_t cellStartIndex = mainGrid->cells().size();
    size_t nodeStartIndex = mainGrid->nodes().size();

    RigCell defaultCell;
    defaultCell.setHostGrid(localGrid);
    mainGrid->cells().resize(cellStartIndex + cellCount, defaultCell);

    mainGrid->nodes().resize(nodeStartIndex + cellCount*8, cvf::Vec3d(0,0,0));

    // Loop over cells and fill them with data

    #pragma omp parallel for
    for (int gIdx = 0; gIdx < cellCount; ++gIdx)
    {
        RigCell& cell = mainGrid->cells()[cellStartIndex + gIdx];

        // The invalid (tainted) cell concept in ecl was not correct at all,
        // so this is disabeled.
        //bool invalid = ecl_grid_cell_invalid1(localEclGrid, gIdx);
        //cell.setInvalid(invalid);

        cell.setCellIndex(gIdx);
        bool active = ecl_grid_cell_active1(localEclGrid, gIdx);
        cell.setActive(active);
        cell.setGlobalActiveIndex(active ? activeStartIndex + ecl_grid_get_active_index1(localEclGrid, gIdx) : cvf::UNDEFINED_SIZE_T);

        int parentCellIndex = ecl_grid_get_parent_cell1(localEclGrid, gIdx);
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
    }
}
#endif

//==================================================================================================
//
// Class RigReaderInterfaceECL
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
RifReaderInterfaceECL::RifReaderInterfaceECL()
{
    ground();
}

//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
RifReaderInterfaceECL::~RifReaderInterfaceECL()
{
}

//--------------------------------------------------------------------------------------------------
/// Ground members
//--------------------------------------------------------------------------------------------------
void RifReaderInterfaceECL::ground()
{
    m_fileName.clear();
    m_fileSet.clear();

    m_staticResults.clear();
    m_dynamicResults.clear();
    m_timeStepTexts.clear();
    m_timeSteps.clear();

    m_numGrids = 0;
}

//--------------------------------------------------------------------------------------------------
/// Close interface (for now, no files are kept open after calling methods, so just clear members)
//--------------------------------------------------------------------------------------------------
void RifReaderInterfaceECL::close()
{
    m_staticResultsAccess     = NULL;
    m_dynamicResultsAccess    = NULL;

    ground();
}

//--------------------------------------------------------------------------------------------------
/// Read geometry from file given by name into given reservoir object
//--------------------------------------------------------------------------------------------------
bool RifReaderInterfaceECL::readGeometry(const QString& filename, RigReservoir* reservoir)
{
    CVF_ASSERT(reservoir);

#ifdef USE_ECL_LIB
    ecl_grid_type * mainEclGrid = ecl_grid_alloc( filename.toAscii().data() );
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

    transferGridCellData(mainGrid, mainGrid, mainEclGrid, 0);

    size_t globalActiveSize = ecl_grid_get_active_size(mainEclGrid);

    for (lgrIdx = 0; lgrIdx < numLGRs; ++lgrIdx)
    {
        ecl_grid_type* localEclGrid = ecl_grid_iget_lgr(mainEclGrid, lgrIdx);
        transferGridCellData(mainGrid, static_cast<RigLocalGrid*>(mainGrid->gridByIndex(lgrIdx+1)), localEclGrid, globalActiveSize);
        globalActiveSize += ecl_grid_get_active_size(localEclGrid);
    }

    ecl_grid_free( mainEclGrid );

    // !! Maybe this should set somewhere else, in a method that builds meta data !!
    m_numGrids = numLGRs + 1;

#endif

    return true;

}

//--------------------------------------------------------------------------------------------------
/// Open file and read geometry into given reservoir object
//--------------------------------------------------------------------------------------------------
bool RifReaderInterfaceECL::open(const QString& fileName, RigReservoir* reservoir)
{
    CVF_ASSERT(reservoir);

    // Make sure everything's closed
    close();

    // Get set of files
    QStringList fileSet;
    if (!RifReaderEclipseFileAccess::fileSet(fileName, &fileSet)) return false;

    // Keep the set of files of interest
    m_fileSet = fileSet;

    // Read geometry
    if (!readGeometry(fileName, reservoir)) return false;

    // Build results meta data
    if (!buildMetaData(reservoir)) return false;

    readWellCells(reservoir);

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Build meta data - get states and results info
//--------------------------------------------------------------------------------------------------
bool RifReaderInterfaceECL::buildMetaData(RigReservoir* reservoir)
{
#ifdef USE_ECL_LIB
    CVF_ASSERT(reservoir);
    CVF_ASSERT(m_fileSet.size() > 0);

    // Get the number of active cells in the grid
    size_t numActiveCells = reservoir->mainGrid()->numActiveCells();

    // Create access object for dynamic results
    m_dynamicResultsAccess = dynamicResultsAccess(m_fileSet, m_numGrids, numActiveCells);
    if (m_dynamicResultsAccess.notNull())
    {
        // Get time steps texts
        m_timeStepTexts = m_dynamicResultsAccess->timeStepsText();
        m_timeSteps = m_dynamicResultsAccess->timeSteps();

        // Get the names of the dynamic results
        m_dynamicResults = m_dynamicResultsAccess->resultNames();
    }

    QString initFileName = RifReaderEclipseFileAccess::fileNameByType(m_fileSet, ECL_INIT_FILE);
    if (initFileName.size() > 0)
    {
        // Open init file
        cvf::ref<RifReaderEclipseFileAccess> initFile = new RifReaderEclipseFileAccess;
        if (!initFile->open(initFileName))
        {
            return false;
        }

        // Get the names of the static results
        QStringList staticResults;
        initFile->keywordsOnFile(&staticResults, numActiveCells);
        m_staticResults = staticResults;

        m_staticResultsAccess = initFile;
    }

    return true;
#else
    return false;
#endif //USE_ECL_LIB
}

//--------------------------------------------------------------------------------------------------
/// Create results access object (.UNRST or .X0001 ... .XNNNN)
//--------------------------------------------------------------------------------------------------
RifReaderEclipseResultsAccess* RifReaderInterfaceECL::dynamicResultsAccess(const QStringList& fileSet, size_t numGrids, size_t numActiveCells)
{
    RifReaderEclipseResultsAccess* resultsAccess = NULL;

#ifdef USE_ECL_LIB
    // Look for unified restart file
    QString unrstFileName = RifReaderEclipseFileAccess::fileNameByType(fileSet, ECL_UNIFIED_RESTART_FILE);
    if (unrstFileName.size() > 0)
    {
        resultsAccess = new RifReaderEclipseUnifiedRestartFile(numGrids, numActiveCells);
        if (!resultsAccess->open(QStringList(unrstFileName)))
        {
            delete resultsAccess;
            return NULL;
        }
    }
    else
    {
        // Look for set of restart files (one file per time step)
        QStringList restartFiles = RifReaderEclipseFileAccess::fileNamesByType(fileSet, ECL_RESTART_FILE);
        if (restartFiles.size() > 0)
        {
            resultsAccess = new RifReaderEclipseRestartFiles(numGrids, numActiveCells);
            if (!resultsAccess->open(restartFiles))
            {
                delete resultsAccess;
                return NULL;
            }
        }
    }
#endif //USE_ECL_LIB

    // !! could add support for formatted result files
    // !! consider priorities in case multiple types exist (.UNRST, .XNNNN, ...)

    return resultsAccess;
}

//--------------------------------------------------------------------------------------------------
/// Get the names of all static results
//--------------------------------------------------------------------------------------------------
const QStringList& RifReaderInterfaceECL::staticResults() const
{
    return m_staticResults;
}

//--------------------------------------------------------------------------------------------------
/// Get the names of all dynamic results
//--------------------------------------------------------------------------------------------------
const QStringList& RifReaderInterfaceECL::dynamicResults() const
{
    return m_dynamicResults;
}

//--------------------------------------------------------------------------------------------------
/// Get the number of time steps
//--------------------------------------------------------------------------------------------------
size_t RifReaderInterfaceECL::numTimeSteps() const
{
    return m_timeStepTexts.size();
}

//--------------------------------------------------------------------------------------------------
/// Get all values of a given static result as doubles
//--------------------------------------------------------------------------------------------------
bool RifReaderInterfaceECL::staticResult(const QString& result, std::vector<double>* values)
{
    CVF_ASSERT(values);
    CVF_ASSERT(m_staticResultsAccess.notNull());

    size_t numOccurrences = m_staticResultsAccess->numOccurrences(result);
    size_t i;
    for (i = 0; i < numOccurrences; i++)
    {
        std::vector<double> partValues;
        if (!m_staticResultsAccess->keywordData(result, i, &partValues)) return false;
        values->insert(values->end(), partValues.begin(), partValues.end());
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Get dynamic result at given step index. Will concatenate values for the main grid and all sub grids.
//--------------------------------------------------------------------------------------------------
bool RifReaderInterfaceECL::dynamicResult(const QString& result, size_t stepIndex, std::vector<double>* values)
{
    CVF_ASSERT(m_dynamicResultsAccess.notNull());
    return m_dynamicResultsAccess->results(result, stepIndex, values);
}

//--------------------------------------------------------------------------------------------------
/// Get list of time step texts
//--------------------------------------------------------------------------------------------------
const QStringList& RifReaderInterfaceECL::timeStepText() const
{
    return m_timeStepTexts;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QList<QDateTime>& RifReaderInterfaceECL::timeSteps() const
{
    return m_timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderInterfaceECL::readWellCells(RigReservoir* reservoir)
{
    CVF_ASSERT(reservoir);

    if (m_dynamicResultsAccess.isNull()) return;

#ifdef USE_ECL_LIB
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
            // Also see RifReaderEclipseFileAccess::timeStepsText for accessing time_t structures
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
                int branchCount = well_state_iget_lgr_num_branches(ert_well_state, gridNr);
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
                const well_conn_type* ert_wellhead = well_state_iget_wellhead(ert_well_state, gridNr);
                if (ert_wellhead)
                {
                    wellResFrame.m_wellHead.m_gridIndex = gridNr;
                    int gridK = CVF_MAX(0, ert_wellhead->k); // Why this ?
                    wellResFrame.m_wellHead.m_gridCellIndex = grids[gridNr]->cellIndexFromIJK(ert_wellhead->i, ert_wellhead->j, gridK);
                }

                int branchCount = well_state_iget_lgr_num_branches(ert_well_state, gridNr);
                if (branchCount > 0)
                {
                    if (static_cast<int>(wellResFrame.m_wellResultBranches.size()) < branchCount) wellResFrame.m_wellResultBranches.resize(branchCount);

                    for (int branchIdx = 0; branchIdx < branchCount; ++branchIdx )
                    {
                        // Connections
                        int connectionCount = well_state_iget_num_lgr_connections(ert_well_state, gridNr, branchIdx);
                        if (connectionCount > 0)
                        {

                            RigWellResultBranch& wellSegment = wellResFrame.m_wellResultBranches[branchIdx]; // Is this completely right? Is the branch index actually the same between lgrs ?
                            wellSegment.m_branchNumber = branchIdx;
                            size_t existingConnCount = wellSegment.m_wellCells.size();
                            wellSegment.m_wellCells.resize(existingConnCount + connectionCount);

                            int connIdx;
                            for (connIdx = 0; connIdx < connectionCount; connIdx++)
                            {
                                const well_conn_type* ert_connection = well_state_iget_lgr_connections( ert_well_state, gridNr, branchIdx)[connIdx];
                                CVF_ASSERT(ert_connection);

                                RigWellResultCell& data = wellSegment.m_wellCells[existingConnCount + connIdx];
                                data.m_gridIndex = gridNr;
                                data.m_gridCellIndex = grids[gridNr]->cellIndexFromIJK(ert_connection->i, ert_connection->j, ert_connection->k);
                                data.m_isOpen = ert_connection->open;
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


#endif
}

