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

#include "RigActiveCellInfo.h"
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
                                                            double                             faultThrow)
{
    // TODO: Remove when we know if LGR can have faults

    size_t anchorI, anchorJ, anchorK;
    mainGrid->ijkFromCellIndex(currentCell, &anchorI, &anchorJ, &anchorK);

    size_t candidate;
    if (mainGrid->cellIJKNeighbor(anchorI, anchorJ, anchorK, face, &candidate))
    {
        appendNeighborCells(allFracturedCells, mainGrid, candidate, connectedCells, faultThrow);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureContainmentTools::checkFaultAndAppendNeighborCell(const std::set<size_t>&            allFracturedCells,
                                                                  const RigMainGrid*                 mainGrid,
                                                                  size_t                             globalReservoirCellIndex,
                                                                  cvf::StructGridInterface::FaceType face,
                                                                  std::set<size_t>&                  connectedCells,
                                                                  double                             faultThrow)
{
    const RigFault* fault = mainGrid->findFaultFromCellIndexAndCellFace(globalReservoirCellIndex, face);
    if (fault)
    {
        if (faultThrow < 0.0)
        {
            return;
        }
        else
        {
            // See RigMainGrid::calculateFaults()

            // TODO: Remove when we know if LGR can have faults

            const RigGridBase* hostGrid = nullptr;

            size_t gridLocalCellIndex;
            hostGrid = mainGrid->gridAndGridLocalIdxFromGlobalCellIdx(globalReservoirCellIndex, &gridLocalCellIndex);

            size_t i, j, k;
            hostGrid->ijkFromCellIndex(gridLocalCellIndex, &i, &j, &k);
            
            size_t neighborGridLocalCellIndex;

            bool foundCell = hostGrid->cellIJKNeighbor(i, j, k, face, &neighborGridLocalCellIndex);
            CVF_ASSERT(foundCell);

            size_t neighborGlobalReservoirCellIndex = hostGrid->reservoirCellIndex(neighborGridLocalCellIndex);

            const RigCell& currentCell = mainGrid->globalCellArray()[globalReservoirCellIndex];
            const RigCell& neightborCell = mainGrid->globalCellArray()[neighborGlobalReservoirCellIndex];

            auto diffBetweenFaceCenters = currentCell.faceCenter(face) - neightborCell.faceCenter(cvf::StructGridInterface::oppositeFace(face));
            if (diffBetweenFaceCenters.length() < faultThrow)
            {
                return;
            }
        }
    }

    appendNeighborCellForFace(allFracturedCells, mainGrid, globalReservoirCellIndex, face, connectedCells, faultThrow);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureContainmentTools::appendNeighborCells(const std::set<size_t>& allFracturedCells,
                                                      const RigMainGrid*      mainGrid,
                                                      size_t                  currentCell,
                                                      std::set<size_t>&       connectedCells,
                                                      double                  faultThrow)
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
    checkFaultAndAppendNeighborCell(allFracturedCells, mainGrid, currentCell, cvf::StructGridInterface::NEG_I, connectedCells, faultThrow);
    checkFaultAndAppendNeighborCell(allFracturedCells, mainGrid, currentCell, cvf::StructGridInterface::POS_I, connectedCells, faultThrow);
    checkFaultAndAppendNeighborCell(allFracturedCells, mainGrid, currentCell, cvf::StructGridInterface::NEG_J, connectedCells, faultThrow);
    checkFaultAndAppendNeighborCell(allFracturedCells, mainGrid, currentCell, cvf::StructGridInterface::POS_J, connectedCells, faultThrow);

    // Append cells without fault check in K direction
    appendNeighborCellForFace(allFracturedCells, mainGrid, currentCell, cvf::StructGridInterface::NEG_K, connectedCells, faultThrow);
    appendNeighborCellForFace(allFracturedCells, mainGrid, currentCell, cvf::StructGridInterface::POS_K, connectedCells, faultThrow);
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
            auto mainGrid       = eclipseCaseData->mainGrid();
            auto activeCellInfo = eclipseCaseData->activeCellInfo(RiaDefines::MATRIX_MODEL);

            if (mainGrid && activeCellInfo)
            {
                std::set<size_t> cellsIntersectingFracturePlane =
                    getCellsIntersectingFracturePlane(mainGrid, activeCellInfo, fracture);

                size_t anchorCellGlobalIndex = fracture->findAnchorEclipseCell(mainGrid);

                // Negative faultThrow disables test on faultThrow
                double faultThrow = -1.0;
                if (fracture && fracture->fractureTemplate())
                {
                    faultThrow = fracture->fractureTemplate()->fractureContainment()->faultThrow();
                }

                appendNeighborCells(
                    cellsIntersectingFracturePlane, mainGrid, anchorCellGlobalIndex, fracturedCellsContainedByFaults, faultThrow);
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
std::set<size_t> RimFractureContainmentTools::getCellsIntersectingFracturePlane(const RigMainGrid*       mainGrid,
                                                                                const RigActiveCellInfo* activeCellInfo,
                                                                                RimFracture*             fracture)
{
    std::set<size_t> eclipseCellIndices;

    {
        const auto indicesToPotentiallyFracturedCells = fracture->getPotentiallyFracturedCells(mainGrid);

        for (const auto& globalCellIndex : indicesToPotentiallyFracturedCells)
        {
            if (activeCellInfo && !activeCellInfo->isActive(globalCellIndex)) continue;

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
