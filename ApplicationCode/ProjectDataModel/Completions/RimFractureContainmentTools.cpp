/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Statoil ASA
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

#include "RimFractureContainmentTools.h"

#include "RigEclipseCaseData.h"
#include "RigFault.h"
#include "RigHexIntersectionTools.h"
#include "RigMainGrid.h"
#include "RigReservoirGridTools.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimFracture.h"
#include "RimFractureContainment.h"
#include "RimFractureTemplate.h"

#include "cvfStructGrid.h"

#include <array>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t findNeighborReservoirCellIndex(const RigMainGrid*                 mainGrid,
                                      cvf::StructGridInterface::FaceType face,
                                      size_t                             globalReservoirCellIndex)
{
    size_t neighborGlobalReservoirCellIndex = cvf::UNDEFINED_SIZE_T;

    if (mainGrid)
    {
        size_t             gridLocalCellIndex = cvf::UNDEFINED_SIZE_T;
        const RigGridBase* hostGrid =
            mainGrid->gridAndGridLocalIdxFromGlobalCellIdx(globalReservoirCellIndex, &gridLocalCellIndex);

        if (hostGrid && gridLocalCellIndex != cvf::UNDEFINED_SIZE_T)
        {
            size_t i, j, k;
            hostGrid->ijkFromCellIndex(gridLocalCellIndex, &i, &j, &k);

            size_t neighborGridLocalCellIndex;

            bool foundCell = hostGrid->cellIJKNeighbor(i, j, k, face, &neighborGridLocalCellIndex);
            if (foundCell)
            {
                neighborGlobalReservoirCellIndex = hostGrid->reservoirCellIndex(neighborGridLocalCellIndex);
            }
        }
    }

    return neighborGlobalReservoirCellIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureContainmentTools::appendNeighborCellForFace(const std::set<size_t>&            allFracturedCells,
                                                            const RigMainGrid*                 mainGrid,
                                                            size_t                             currentCell,
                                                            cvf::StructGridInterface::FaceType face,
                                                            std::set<size_t>&                  connectedCells,
                                                            double                             minimumFaultThrow)
{
    size_t candidate = findNeighborReservoirCellIndex(mainGrid, face, currentCell);
    if (candidate != cvf::UNDEFINED_SIZE_T)
    {
        appendNeighborCells(allFracturedCells, mainGrid, candidate, connectedCells, minimumFaultThrow);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double computeAverageZForTwoDeepestZ(const RigMainGrid*                 mainGrid,
                                     size_t                             globalReservoirCellIndex,
                                     cvf::StructGridInterface::FaceType face)
{
    cvf::Vec3d hexCorners[8];
    mainGrid->cellCornerVertices(globalReservoirCellIndex, hexCorners);

    double avgZ = 0.0;

    cvf::ubyte faceVertexIndices[4];
    cvf::StructGridInterface::cellFaceVertexIndices(face, faceVertexIndices);

    for (const auto& faceIdx : faceVertexIndices)
    {
        // Face indices 0-3 are defined to have deepest Z
        // See void StructGridInterface::cellFaceVertexIndices(FaceType face, cvf::ubyte vertexIndices[4])

        if (faceIdx < 4)
        {
            avgZ += hexCorners[faceIdx].z();
        }
    }

    avgZ /= 2.0;

    return avgZ;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureContainmentTools::checkFaultAndAppendNeighborCell(const std::set<size_t>&            allFracturedCells,
                                                                  const RigMainGrid*                 mainGrid,
                                                                  size_t                             globalReservoirCellIndex,
                                                                  cvf::StructGridInterface::FaceType face,
                                                                  std::set<size_t>&                  connectedCells,
                                                                  double                             minimumFaultThrow)
{
    const RigFault* fault = mainGrid->findFaultFromCellIndexAndCellFace(globalReservoirCellIndex, face);
    if (fault)
    {
        {
            // See RigMainGrid::calculateFaults() for reference

            // This function is intended to support fractures in LGR-grids
            // Currently, only faults in main grid is supported when reading fault specifications from input text files
            // Eclipse 300 supports faults in LGR
            // https://github.com/OPM/ResInsight/issues/3019

            size_t neighborGlobalReservoirCellIndex = findNeighborReservoirCellIndex(mainGrid, face, globalReservoirCellIndex);
            if (neighborGlobalReservoirCellIndex == cvf::UNDEFINED_SIZE_T)
            {
                // This is probably an assert condition, but we return directly to ensure we are robust
                return;
            }

            double currentCellAvgZ  = computeAverageZForTwoDeepestZ(mainGrid, globalReservoirCellIndex, face);
            double neighborCellAvgZ = computeAverageZForTwoDeepestZ(
                mainGrid, neighborGlobalReservoirCellIndex, cvf::StructGridInterface::oppositeFace(face));

            double faultThrow = fabs(currentCellAvgZ - neighborCellAvgZ);
            if (faultThrow > minimumFaultThrow)
            {
                return;
            }
        }
    }

    appendNeighborCellForFace(allFracturedCells, mainGrid, globalReservoirCellIndex, face, connectedCells, minimumFaultThrow);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureContainmentTools::appendNeighborCells(const std::set<size_t>& allFracturedCells,
                                                      const RigMainGrid*      mainGrid,
                                                      size_t                  currentCell,
                                                      std::set<size_t>&       connectedCells,
                                                      double                  minimumFaultThrow)
{
    if (std::find(connectedCells.begin(), connectedCells.end(), currentCell) != connectedCells.end())
    {
        // currentCell is already handled
        return;
    }

    if (std::find(allFracturedCells.begin(), allFracturedCells.end(), currentCell) == allFracturedCells.end())
    {
        // currentCell is not found among the set of fracture cells
        return;
    }

    connectedCells.insert(currentCell);

    // Check faults in IJ directions
    checkFaultAndAppendNeighborCell(
        allFracturedCells, mainGrid, currentCell, cvf::StructGridInterface::NEG_I, connectedCells, minimumFaultThrow);
    checkFaultAndAppendNeighborCell(
        allFracturedCells, mainGrid, currentCell, cvf::StructGridInterface::POS_I, connectedCells, minimumFaultThrow);
    checkFaultAndAppendNeighborCell(
        allFracturedCells, mainGrid, currentCell, cvf::StructGridInterface::NEG_J, connectedCells, minimumFaultThrow);
    checkFaultAndAppendNeighborCell(
        allFracturedCells, mainGrid, currentCell, cvf::StructGridInterface::POS_J, connectedCells, minimumFaultThrow);

    // Append cells without fault check in K direction
    appendNeighborCellForFace(
        allFracturedCells, mainGrid, currentCell, cvf::StructGridInterface::NEG_K, connectedCells, minimumFaultThrow);
    appendNeighborCellForFace(
        allFracturedCells, mainGrid, currentCell, cvf::StructGridInterface::POS_K, connectedCells, minimumFaultThrow);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<size_t> RimFractureContainmentTools::reservoirCellIndicesOpenForFlow(const RimEclipseCase* eclipseCase,
                                                                              const RimFracture*    fracture)
{
    std::set<size_t> cellsOpenForFlow;

    if (eclipseCase && fracture)
    {
        auto eclipseCaseData = eclipseCase->eclipseCaseData();
        if (eclipseCaseData)
        {
            auto mainGrid = eclipseCaseData->mainGrid();

            if (mainGrid)
            {
                std::set<size_t> cellsIntersectingFracturePlane = getCellsIntersectingFracturePlane(mainGrid, fracture);

                // Negative faultThrow disables test on faultThrow
                double maximumFaultThrow = -1.0;
                if (fracture->fractureTemplate() && fracture->fractureTemplate()->fractureContainment())
                {
                    maximumFaultThrow = fracture->fractureTemplate()->fractureContainment()->minimumFaultThrow();
                }

                if (maximumFaultThrow > -1.0)
                {
                    size_t anchorCellGlobalIndex = mainGrid->findReservoirCellIndexFromPoint(fracture->anchorPosition());
                    appendNeighborCells(cellsIntersectingFracturePlane,
                                        mainGrid,
                                        anchorCellGlobalIndex,
                                        cellsOpenForFlow,
                                        maximumFaultThrow);
                }
                else
                {
                    cellsOpenForFlow = cellsIntersectingFracturePlane;
                }
            }

            /*
            NB : Please do not delete this code, used to create input to cell based range filter to see the computed fracture
            cells

                        qDebug() << "FracturedCells - Truncated";
                        qDebug() << RigReservoirGridTools::globalCellIndicesToOneBasedIJKText(
                            fracturedCellsContainedByFaults.begin(), fracturedCellsContainedByFaults.end(), mainGrid);
            */
        }
    }

    return cellsOpenForFlow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<size_t> RimFractureContainmentTools::getCellsIntersectingFracturePlane(const RigMainGrid* mainGrid,
                                                                                const RimFracture* fracture)
{
    std::set<size_t> eclipseCellIndices;

    {
        const auto indicesToPotentiallyFracturedCells = fracture->getPotentiallyFracturedCells(mainGrid);

        for (const auto& globalCellIndex : indicesToPotentiallyFracturedCells)
        {
            std::array<cvf::Vec3d, 8> hexCorners;
            mainGrid->cellCornerVertices(globalCellIndex, hexCorners.data());
            std::vector<std::vector<cvf::Vec3d>> planeCellPolygons;

            bool isPlanIntersected =
                RigHexIntersectionTools::planeHexIntersectionPolygons(hexCorners, fracture->transformMatrix(), planeCellPolygons);
            if (isPlanIntersected || !planeCellPolygons.empty())
            {
                eclipseCellIndices.insert(globalCellIndex);
            }
        }
    }

    return eclipseCellIndices;
}
