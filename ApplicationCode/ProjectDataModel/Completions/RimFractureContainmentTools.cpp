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
void RimFractureContainmentTools::appendNeighborCellForFace(const std::set<size_t>&            allFracturedCells,
                                                            const RigMainGrid*                 mainGrid,
                                                            size_t                             currentCell,
                                                            cvf::StructGridInterface::FaceType face,
                                                            std::set<size_t>&                  connectedCells,
                                                            double                             maximumFaultThrow)
{
    // TODO: Remove when we know if LGR can have faults

    size_t anchorI, anchorJ, anchorK;
    mainGrid->ijkFromCellIndex(currentCell, &anchorI, &anchorJ, &anchorK);

    size_t candidate;
    if (mainGrid->cellIJKNeighbor(anchorI, anchorJ, anchorK, face, &candidate))
    {
        appendNeighborCells(allFracturedCells, mainGrid, candidate, connectedCells, maximumFaultThrow);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double computeAverageZFromTwoDeepestZ(const RigMainGrid*                 mainGrid,
                                      size_t                             globalReservoirCellIndex,
                                      cvf::StructGridInterface::FaceType face)
{
    const RigCell& currentCell = mainGrid->globalCellArray()[globalReservoirCellIndex];

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
                                                                  double                             maximumFaultThrow)
{
    const RigFault* fault = mainGrid->findFaultFromCellIndexAndCellFace(globalReservoirCellIndex, face);
    if (fault)
    {
        if (maximumFaultThrow < 0.0)
        {
            return;
        }
        else
        {
            // See RigMainGrid::calculateFaults()

            size_t neighborGlobalReservoirCellIndex = 0;
            {
                const RigGridBase* hostGrid = nullptr;
                size_t             gridLocalCellIndex;
                hostGrid = mainGrid->gridAndGridLocalIdxFromGlobalCellIdx(globalReservoirCellIndex, &gridLocalCellIndex);
                CVF_ASSERT(hostGrid);

                size_t i, j, k;
                hostGrid->ijkFromCellIndex(gridLocalCellIndex, &i, &j, &k);

                size_t neighborGridLocalCellIndex;

                bool foundCell = hostGrid->cellIJKNeighbor(i, j, k, face, &neighborGridLocalCellIndex);
                CVF_ASSERT(foundCell);

                neighborGlobalReservoirCellIndex =
                    mainGrid->reservoirCellIndexByGridAndGridLocalCellIndex(hostGrid->gridIndex(), neighborGridLocalCellIndex);
            }

            double currentCellAvgZ  = computeAverageZFromTwoDeepestZ(mainGrid, globalReservoirCellIndex, face);
            double neighborCellAvgZ = computeAverageZFromTwoDeepestZ(
                mainGrid, neighborGlobalReservoirCellIndex, cvf::StructGridInterface::oppositeFace(face));

            double faultThrow = fabs(currentCellAvgZ - neighborCellAvgZ);
            if (faultThrow > maximumFaultThrow)
            {
                return;
            }
        }
    }

    appendNeighborCellForFace(allFracturedCells, mainGrid, globalReservoirCellIndex, face, connectedCells, maximumFaultThrow);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureContainmentTools::appendNeighborCells(const std::set<size_t>& allFracturedCells,
                                                      const RigMainGrid*      mainGrid,
                                                      size_t                  currentCell,
                                                      std::set<size_t>&       connectedCells,
                                                      double                  maximumFaultThrow)
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
        allFracturedCells, mainGrid, currentCell, cvf::StructGridInterface::NEG_I, connectedCells, maximumFaultThrow);
    checkFaultAndAppendNeighborCell(
        allFracturedCells, mainGrid, currentCell, cvf::StructGridInterface::POS_I, connectedCells, maximumFaultThrow);
    checkFaultAndAppendNeighborCell(
        allFracturedCells, mainGrid, currentCell, cvf::StructGridInterface::NEG_J, connectedCells, maximumFaultThrow);
    checkFaultAndAppendNeighborCell(
        allFracturedCells, mainGrid, currentCell, cvf::StructGridInterface::POS_J, connectedCells, maximumFaultThrow);

    // Append cells without fault check in K direction
    appendNeighborCellForFace(
        allFracturedCells, mainGrid, currentCell, cvf::StructGridInterface::NEG_K, connectedCells, maximumFaultThrow);
    appendNeighborCellForFace(
        allFracturedCells, mainGrid, currentCell, cvf::StructGridInterface::POS_K, connectedCells, maximumFaultThrow);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<size_t> RimFractureContainmentTools::fracturedCellsTruncatedByFaults(const RimEclipseCase* eclipseCase,
                                                                              RimFracture*          fracture)
{
    std::set<size_t> fracturedCellsContainedByFaults;

    if (eclipseCase && fracture)
    {
        auto eclipseCaseData = eclipseCase->eclipseCaseData();
        if (eclipseCaseData)
        {
            auto mainGrid = eclipseCaseData->mainGrid();

            if (mainGrid)
            {
                std::set<size_t> cellsIntersectingFracturePlane = getCellsIntersectingFracturePlane(mainGrid, fracture);

                size_t anchorCellGlobalIndex = fracture->findAnchorEclipseCell(mainGrid);

                // Negative faultThrow disables test on faultThrow
                double maximumFaultThrow = -1.0;
                if (fracture && fracture->fractureTemplate())
                {
                    maximumFaultThrow = fracture->fractureTemplate()->fractureContainment()->maximumFaultThrow();
                }

                appendNeighborCells(cellsIntersectingFracturePlane,
                                    mainGrid,
                                    anchorCellGlobalIndex,
                                    fracturedCellsContainedByFaults,
                                    maximumFaultThrow);
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

    return fracturedCellsContainedByFaults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<size_t> RimFractureContainmentTools::getCellsIntersectingFracturePlane(const RigMainGrid* mainGrid,
                                                                                RimFracture*       fracture)
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
