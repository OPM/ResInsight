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

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimFracture.h"

#include "cvfStructGrid.h"

#include <array>

#include <QDebug>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureContainmentTools::appendNeighborCellForFace(const std::set<size_t>&            allFracturedCells,
                                                            const RigMainGrid*                 mainGrid,
                                                            size_t                             currentCell,
                                                            cvf::StructGridInterface::FaceType face,
                                                            std::set<size_t>&                  connectedCells)
{
    size_t anchorI, anchorJ, anchorK;
    mainGrid->ijkFromCellIndex(currentCell, &anchorI, &anchorJ, &anchorK);

    size_t candidate;
    if (mainGrid->cellIJKNeighbor(anchorI, anchorJ, anchorK, face, &candidate))
    {
        if (std::find(allFracturedCells.begin(), allFracturedCells.end(), candidate) != allFracturedCells.end())
        {
            if (std::find(connectedCells.begin(), connectedCells.end(), candidate) == connectedCells.end())
            {
                connectedCells.insert(candidate);

                appendNeighborCells(allFracturedCells, mainGrid, candidate, connectedCells);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureContainmentTools::checkFaultAndAppendNeighborCell(const std::set<size_t>&            allFracturedCells,
                                                                  const RigMainGrid*                 mainGrid,
                                                                  size_t                             currentCell,
                                                                  cvf::StructGridInterface::FaceType face,
                                                                  std::set<size_t>&                  connectedCells)
{
    const RigFault* fault = mainGrid->findFaultFromCellIndexAndCellFace(currentCell, face);
    if (fault)
    {
        size_t i, j, k;
        mainGrid->ijkFromCellIndex(currentCell, &i, &j, &k);

        qDebug() << QString("Found cell at ijk %1").arg(i).arg(j).arg(k);
        return;
    }

    appendNeighborCellForFace(allFracturedCells, mainGrid, currentCell, cvf::StructGridInterface::NEG_I, connectedCells);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureContainmentTools::appendNeighborCells(const std::set<size_t>& allFracturedCells,
                                                      const RigMainGrid*      mainGrid,
                                                      size_t                  currentCell,
                                                      std::set<size_t>&       connectedCells)
{
    // Stop at faults in IJ directions
    checkFaultAndAppendNeighborCell(allFracturedCells, mainGrid, currentCell, cvf::StructGridInterface::NEG_I, connectedCells);
    checkFaultAndAppendNeighborCell(allFracturedCells, mainGrid, currentCell, cvf::StructGridInterface::POS_I, connectedCells);
    checkFaultAndAppendNeighborCell(allFracturedCells, mainGrid, currentCell, cvf::StructGridInterface::NEG_J, connectedCells);
    checkFaultAndAppendNeighborCell(allFracturedCells, mainGrid, currentCell, cvf::StructGridInterface::POS_J, connectedCells);

    // Append cells without fault check in K direction
    appendNeighborCellForFace(allFracturedCells, mainGrid, currentCell, cvf::StructGridInterface::NEG_K, connectedCells);
    appendNeighborCellForFace(allFracturedCells, mainGrid, currentCell, cvf::StructGridInterface::POS_K, connectedCells);
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
                std::set<size_t> allFracturedCells = getFracturedCells(mainGrid, activeCellInfo, fracture);

                size_t anchorCellGlobalIndex = fracture->findAnchorEclipseCell(mainGrid);

                appendNeighborCells(allFracturedCells, mainGrid, anchorCellGlobalIndex, fracturedCellsContainedByFaults);
            }
        }
    }

    return fracturedCellsContainedByFaults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<size_t> RimFractureContainmentTools::getFracturedCells(const RigMainGrid*       mainGrid,
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
