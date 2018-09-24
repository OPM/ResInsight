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

#include "RigReservoirBuilderMock.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigCell.h"
#include "RigMainGrid.h"
#include "RigSimWellData.h"

/* rand example: guess the number */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigReservoirBuilderMock::RigReservoirBuilderMock()
{
    m_resultCount = 0;
    m_timeStepCount = 0;
    m_gridPointDimensions = cvf::Vec3st::ZERO;
    m_enableWellData = true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilderMock::setGridPointDimensions(const cvf::Vec3st& gridPointDimensions)
{
    m_gridPointDimensions = gridPointDimensions;       
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilderMock::setResultInfo(size_t resultCount, size_t timeStepCount)
{
    m_resultCount = resultCount;
    m_timeStepCount = timeStepCount;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilderMock::appendNodes(const cvf::Vec3d& min, const cvf::Vec3d& max, const cvf::Vec3st& cubeDimension, std::vector<cvf::Vec3d>& nodes)
{
    double dx = (max.x() - min.x()) / static_cast<double>(cubeDimension.x());
    double dy = (max.y() - min.y()) / static_cast<double>(cubeDimension.y());
    double dz = (max.z() - min.z()) / static_cast<double>(cubeDimension.z());

    double zPos = min.z();

    size_t k;
    for (k = 0; k < cubeDimension.z(); k++)
    {
        double yPos = min.y();

        size_t j;
        for (j = 0; j < cubeDimension.y(); j++)
        {
            double xPos = min.x();

            size_t i;
            for (i = 0; i < cubeDimension.x(); i++)
            {
                cvf::Vec3d cornerA(xPos, yPos, zPos);
                cvf::Vec3d cornerB(xPos + dx, yPos + dy, zPos + dz);

                appendCubeNodes(cornerA, cornerB, nodes);

                xPos += dx;
            }

            yPos += dy;
        }

        zPos += dz;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilderMock::appendCubeNodes(const cvf::Vec3d& min, const cvf::Vec3d& max, std::vector<cvf::Vec3d>& nodes)
{
    //
    //     7---------6                Faces:
    //    /|        /|     |k           0 bottom   0, 3, 2, 1
    //   / |       / |     | /j         1 top      4, 5, 6, 7
    //  4---------5  |     |/           2 front    0, 1, 5, 4
    //  |  3------|--2     *---i        3 right    1, 2, 6, 5
    //  | /       | /                   4 back     3, 7, 6, 2
    //  |/        |/                    5 left     0, 4, 7, 3
    //  0---------1                     

    cvf::Vec3d v0(min.x(), min.y(), min.z());
    cvf::Vec3d v1(max.x(), min.y(), min.z());
    cvf::Vec3d v2(max.x(), max.y(), min.z());
    cvf::Vec3d v3(min.x(), max.y(), min.z());
             
    cvf::Vec3d v4(min.x(), min.y(), max.z());
    cvf::Vec3d v5(max.x(), min.y(), max.z());
    cvf::Vec3d v6(max.x(), max.y(), max.z());
    cvf::Vec3d v7(min.x(), max.y(), max.z());

    nodes.push_back(v0);
    nodes.push_back(v1);
    nodes.push_back(v2);
    nodes.push_back(v3);
    nodes.push_back(v4);
    nodes.push_back(v5);
    nodes.push_back(v6);
    nodes.push_back(v7);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilderMock::appendCells(size_t nodeStartIndex, size_t cellCount, RigGridBase* hostGrid, std::vector<RigCell>& cells)
{
    long long i;

    size_t cellIndexStart = cells.size();
    cells.resize(cells.size() + cellCount);

#pragma omp parallel for
    for (i = 0; i < static_cast<long long>(cellCount); i++)
    {
        RigCell& riCell = cells[cellIndexStart + i];

        riCell.setHostGrid(hostGrid);
        riCell.setGridLocalCellIndex(i);

        riCell.cornerIndices()[0] = nodeStartIndex + i * 8 + 0;
        riCell.cornerIndices()[1] = nodeStartIndex + i * 8 + 1;
        riCell.cornerIndices()[2] = nodeStartIndex + i * 8 + 2;
        riCell.cornerIndices()[3] = nodeStartIndex + i * 8 + 3;
        riCell.cornerIndices()[4] = nodeStartIndex + i * 8 + 4;
        riCell.cornerIndices()[5] = nodeStartIndex + i * 8 + 5;
        riCell.cornerIndices()[6] = nodeStartIndex + i * 8 + 6;
        riCell.cornerIndices()[7] = nodeStartIndex + i * 8 + 7;

        riCell.setParentCellIndex(0);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilderMock::populateReservoir(RigEclipseCaseData* eclipseCase)
{
    std::vector<cvf::Vec3d>& mainGridNodes = eclipseCase->mainGrid()->nodes();
    appendNodes(m_minWorldCoordinate, m_maxWorldCoordinate, cellDimension(), mainGridNodes);
    size_t mainGridNodeCount = mainGridNodes.size();
    size_t mainGridCellCount = mainGridNodeCount / 8;

    // Must create cells in main grid here, as this information is used when creating LGRs
    appendCells(0, mainGridCellCount, eclipseCase->mainGrid(), eclipseCase->mainGrid()->globalCellArray());

    size_t totalCellCount = mainGridCellCount;

    size_t lgrIdx;
    for (lgrIdx = 0; lgrIdx < m_localGridRefinements.size(); lgrIdx++)
    {
        LocalGridRefinement& lgr = m_localGridRefinements[lgrIdx];

        // Compute all global cell indices to be replaced by local grid refinement
        std::vector<size_t> mainGridIndicesWithSubGrid;
        {
            size_t i;
            for (i = lgr.m_mainGridMinCellPosition.x(); i <= lgr.m_mainGridMaxCellPosition.x(); i++)
            {
                size_t j;
                for (j = lgr.m_mainGridMinCellPosition.y(); j <= lgr.m_mainGridMaxCellPosition.y(); j++)
                {
                    size_t k;
                    for (k = lgr.m_mainGridMinCellPosition.z(); k <= lgr.m_mainGridMaxCellPosition.z(); k++)
                    {
                        mainGridIndicesWithSubGrid.push_back(cellIndexFromIJK(i, j, k));
                    }
                }
            }
        }

        // Create local grid and set local grid dimensions
        RigLocalGrid* localGrid = new RigLocalGrid(eclipseCase->mainGrid());
        localGrid->setGridId(1);
        eclipseCase->mainGrid()->addLocalGrid(localGrid);
        localGrid->setParentGrid(eclipseCase->mainGrid());
        
        localGrid->setIndexToStartOfCells(mainGridNodes.size() / 8);
        cvf::Vec3st gridPointDimensions(
            lgr.m_singleCellRefinementFactors.x() * (lgr.m_mainGridMaxCellPosition.x() - lgr.m_mainGridMinCellPosition.x() + 1) + 1,
            lgr.m_singleCellRefinementFactors.y() * (lgr.m_mainGridMaxCellPosition.y() - lgr.m_mainGridMinCellPosition.y() + 1) + 1,
            lgr.m_singleCellRefinementFactors.z() * (lgr.m_mainGridMaxCellPosition.z() - lgr.m_mainGridMinCellPosition.z() + 1) + 1);
        localGrid->setGridPointDimensions(gridPointDimensions);

        cvf::BoundingBox bb;
        size_t cellIdx;
        for (cellIdx = 0; cellIdx < mainGridIndicesWithSubGrid.size(); cellIdx++)
        {
            RigCell& cell = eclipseCase->mainGrid()->globalCellArray()[mainGridIndicesWithSubGrid[cellIdx]];
            
            std::array<size_t, 8>& indices = cell.cornerIndices();
            int nodeIdx;
            for (nodeIdx = 0; nodeIdx < 8; nodeIdx++)
            {
                bb.add(eclipseCase->mainGrid()->nodes()[indices[nodeIdx]]);
            }
            // Deactivate cell in main grid
            cell.setSubGrid(localGrid);
        }

        cvf::Vec3st lgrCellDimensions = gridPointDimensions - cvf::Vec3st(1, 1, 1);
        appendNodes(bb.min(), bb.max(), lgrCellDimensions, mainGridNodes);

        size_t subGridCellCount = (mainGridNodes.size() / 8) - totalCellCount;
        appendCells(totalCellCount*8, subGridCellCount, localGrid, eclipseCase->mainGrid()->globalCellArray());
        totalCellCount += subGridCellCount;
    }

    eclipseCase->mainGrid()->setGridPointDimensions(m_gridPointDimensions);

    if (m_enableWellData)
    {
        addWellData(eclipseCase, eclipseCase->mainGrid());
    }

    addFaults(eclipseCase);

    // Set all cells active
    RigActiveCellInfo* activeCellInfo = eclipseCase->activeCellInfo(RiaDefines::MATRIX_MODEL);
    activeCellInfo->setReservoirCellCount(eclipseCase->mainGrid()->globalCellArray().size());
    for (size_t i = 0; i < eclipseCase->mainGrid()->globalCellArray().size(); i++)
    {
        activeCellInfo->setCellResultIndex(i, i);
    }

    activeCellInfo->setGridCount(1);
    activeCellInfo->setGridActiveCellCounts(0, eclipseCase->mainGrid()->globalCellArray().size());
    activeCellInfo->computeDerivedData();

    // Add grid coarsening for main grid
    if (cellDimension().x() > 4 &&
        cellDimension().y() > 5 &&
        cellDimension().z() > 6)
    {
        eclipseCase->mainGrid()->addCoarseningBox(1, 2, 1, 3, 1, 4);
        eclipseCase->mainGrid()->addCoarseningBox(3, 4, 4, 5, 5, 6);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilderMock::addLocalGridRefinement(const cvf::Vec3st& mainGridStart, const cvf::Vec3st& mainGridEnd, const cvf::Vec3st& refinementFactors)
{
    m_localGridRefinements.push_back(LocalGridRefinement(mainGridStart, mainGridEnd, refinementFactors));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilderMock::setWorldCoordinates(cvf::Vec3d minWorldCoordinate, cvf::Vec3d maxWorldCoordinate)
{
    m_minWorldCoordinate = minWorldCoordinate;
    m_maxWorldCoordinate = maxWorldCoordinate;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigReservoirBuilderMock::inputProperty(RigEclipseCaseData* eclipseCase, const QString& propertyName, std::vector<double>* values)
{
    size_t k;

    /* initialize random seed: */
    srand ( time(nullptr) );

    /* generate secret number: */
    int iSecret = rand() % 20 + 1;

    for (k = 0; k < eclipseCase->mainGrid()->globalCellArray().size(); k++)
    {
        values->push_back(k * iSecret);
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigReservoirBuilderMock::staticResult(RigEclipseCaseData* eclipseCase, const QString& result, std::vector<double>* values)
{
    values->resize(eclipseCase->mainGrid()->globalCellArray().size());

#pragma omp parallel for
    for (long long k = 0; k < static_cast<long long>(eclipseCase->mainGrid()->globalCellArray().size()); k++)
    {
        values->at(k) = (k * 2) % eclipseCase->mainGrid()->globalCellArray().size();
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigReservoirBuilderMock::dynamicResult(RigEclipseCaseData* eclipseCase, const QString& result, size_t stepIndex, std::vector<double>* values)
{
    int resultIndex = 1;

    QRegExp rx("[0-9]{1,2}");   // Find number 0-99
    int digitPos = rx.indexIn(result);
    if (digitPos > -1)
    {
        resultIndex = rx.cap(0).toInt() + 1;
    }

    double scaleValue = 1.0 + resultIndex * 0.1;
    double offsetValue = 100 * resultIndex;

    values->resize(eclipseCase->mainGrid()->globalCellArray().size());

#pragma omp parallel for
    for (long long k = 0; k < static_cast<long long>(eclipseCase->mainGrid()->globalCellArray().size()); k++)
    {
        double val = offsetValue + scaleValue * ( (stepIndex * 1000 + k) % eclipseCase->mainGrid()->globalCellArray().size() );
        values->at(k) = val;
    }

    // Set result size to zero for some timesteps
    if ((stepIndex + 1) % 3 == 0)
    {
        values->clear();
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilderMock::addWellData(RigEclipseCaseData* eclipseCase, RigGridBase* grid)
{
    CVF_ASSERT(eclipseCase);
    CVF_ASSERT(grid);

    cvf::Vec3st dim = grid->gridPointDimensions();

    cvf::Collection<RigSimWellData> wells;

    int wellIdx;
    for (wellIdx = 0; wellIdx < 1; wellIdx++)
    {
        cvf::ref<RigSimWellData> wellCellsTimeHistory = new RigSimWellData;
        wellCellsTimeHistory->m_wellName = QString("Well %1").arg(wellIdx);

        wellCellsTimeHistory->m_wellCellsTimeSteps.resize(m_timeStepCount);

        size_t timeIdx;
        for (timeIdx = 0; timeIdx < m_timeStepCount; timeIdx++)
        {
            RigWellResultFrame& wellCells = wellCellsTimeHistory->m_wellCellsTimeSteps[timeIdx];

            wellCells.m_productionType = RigWellResultFrame::PRODUCER;
            wellCells.m_isOpen = true;

            wellCells.m_wellHead.m_gridIndex = 0;
            wellCells.m_wellHead.m_gridCellIndex = grid->cellIndexFromIJK(1, 0, 0);

            // Connections
//            int connectionCount = CVF_MIN(dim.x(), CVF_MIN(dim.y(), dim.z())) - 2;
            size_t connectionCount = dim.z() - 2;
            if (connectionCount > 0)
            {
                // Only main grid supported by now. Must be taken care of when LGRs are supported
                wellCells.m_wellResultBranches.resize(1);
                RigWellResultBranch& wellSegment = wellCells.m_wellResultBranches[0];

                size_t connIdx;
                for (connIdx = 0; connIdx < connectionCount; connIdx++)
                {
                    if (connIdx == (size_t)(connectionCount/4)) continue;

                    RigWellResultPoint data;
                    data.m_gridIndex = 0;

                    if (connIdx < dim.y()-2)
                        data.m_gridCellIndex = grid->cellIndexFromIJK(1 , 1 + connIdx , 1 + connIdx);
                    else
                        data.m_gridCellIndex = grid->cellIndexFromIJK(1 , dim.y()-2 , 1 + connIdx);
                   

                    if (connIdx < connectionCount / 2)
                    {
                        data.m_isOpen = true;
                    }
                    else
                    {
                        data.m_isOpen = false;
                    }

                    if (wellSegment.m_branchResultPoints.size() == 0 || wellSegment.m_branchResultPoints.back().m_gridCellIndex != data.m_gridCellIndex)
                    {
                        wellSegment.m_branchResultPoints.push_back(data);

                        if (connIdx == connectionCount / 2 )
                        {
                            RigWellResultPoint deadEndData = data;
                            deadEndData.m_gridCellIndex = data.m_gridCellIndex + 1;
                            deadEndData.m_isOpen = true;

                            RigWellResultPoint deadEndData1 = data;
                            deadEndData1.m_gridCellIndex = data.m_gridCellIndex + 2;
                            deadEndData1.m_isOpen = false;

                            wellSegment.m_branchResultPoints.push_back(deadEndData);
                            wellSegment.m_branchResultPoints.push_back(deadEndData1);

                            wellSegment.m_branchResultPoints.push_back(deadEndData);

                            data.m_isOpen = true;
                            wellSegment.m_branchResultPoints.push_back(data);
                        }
                    }

                    if (connIdx < dim.y()-2)
                    {
                        data.m_gridCellIndex =  grid->cellIndexFromIJK(1 , 1 + connIdx , 2 + connIdx);

                        if (wellSegment.m_branchResultPoints.size() == 0 || wellSegment.m_branchResultPoints.back().m_gridCellIndex != data.m_gridCellIndex)
                        {
                            wellSegment.m_branchResultPoints.push_back(data);
                        }
                    }
                }
            }
        }

        // Create a mapping from result timestep indices to well timestep indices.
        // Use one-to-one mapping for easy use
        std::vector<size_t> map;
        for (timeIdx = 0; timeIdx < m_timeStepCount; timeIdx++)
        {
            map.push_back(timeIdx);
        }
        wellCellsTimeHistory->m_resultTimeStepIndexToWellTimeStepIndex = map;

        wells.push_back(wellCellsTimeHistory.p());
    }

    eclipseCase->setSimWellData(wells);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilderMock::addFaults(RigEclipseCaseData* eclipseCase)
{
    if (!eclipseCase) return;

    RigMainGrid* grid = eclipseCase->mainGrid();
    if (!grid) return;

    cvf::Collection<RigFault> faults;

    {
        cvf::ref<RigFault> fault = new RigFault;
        fault->setName("Fault A");
        
        cvf::Vec3st min = cvf::Vec3st::ZERO;
        cvf::Vec3st max(0, 0, cellDimension().z() - 2);
        
        if (cellDimension().x() > 5)
        {
            min.x() = cellDimension().x() / 2;
            max.x() = min.x() + 2;
        }
        
        if (cellDimension().y() > 5)
        {
            min.y() = cellDimension().y() / 2;
            max.y() = cellDimension().y() / 2;
        }

        cvf::CellRange cellRange(min, max);

        fault->addCellRangeForFace(cvf::StructGridInterface::POS_I, cellRange);
        faults.push_back(fault.p());
    }

    grid->setFaults(faults);

    // NNCs
    std::vector<RigConnection>& nncConnections = grid->nncData()->connections();
    {
        size_t i1 = 2;
        size_t j1 = 2;
        size_t k1 = 3;
        
        size_t i2 = 2;
        size_t j2 = 3;
        size_t k2 = 4;

        addNnc(grid, i1, j1, k1, i2, j2, k2, nncConnections);
    }

    {
        size_t i1 = 2;
        size_t j1 = 2;
        size_t k1 = 3;

        size_t i2 = 2;
        size_t j2 = 1;
        size_t k2 = 4;

        addNnc(grid, i1, j1, k1, i2, j2, k2, nncConnections);
    }

    std::vector<double>& tranVals = grid->nncData()->makeStaticConnectionScalarResult(RigNNCData::propertyNameCombTrans());
    for (size_t cIdx = 0; cIdx < tranVals.size(); ++cIdx)
    {
        tranVals[cIdx] = 0.2;
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilderMock::enableWellData(bool enableWellData)
{
    m_enableWellData = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilderMock::addNnc(RigMainGrid* grid, size_t i1, size_t j1, size_t k1, size_t i2, size_t j2, size_t k2, std::vector<RigConnection> &nncConnections)
{
    size_t c1GlobalIndex = grid->cellIndexFromIJK(i1, j1, k1);
    size_t c2GlobalIndex = grid->cellIndexFromIJK(i2, j2, k2);

    RigConnection conn;
    conn.m_c1GlobIdx = c1GlobalIndex;
    conn.m_c2GlobIdx = c2GlobalIndex;

    nncConnections.push_back(conn);
}

