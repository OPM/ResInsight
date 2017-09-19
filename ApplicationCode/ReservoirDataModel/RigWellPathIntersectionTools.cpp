/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RigWellPathIntersectionTools.h"

#include "RiaLogging.h"

#include "RigWellPath.h"
#include "RigMainGrid.h"
#include "RigEclipseCaseData.h"
#include "RigWellLogExtractionTools.h"
#include "RigCellGeometryTools.h"

#include "cvfGeometryTools.h"
#include "cvfMatrix3.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<WellPathCellIntersectionInfo> RigWellPathIntersectionTools::findCellsIntersectedByPath(const RigEclipseCaseData* caseData, 
                                                                                                   const std::vector<cvf::Vec3d>& pathCoords)
{
    std::vector<WellPathCellIntersectionInfo> intersectionInfos;
    const RigMainGrid* grid = caseData->mainGrid();

    if (pathCoords.size() < 2) return intersectionInfos;

    std::vector<HexIntersectionInfo> intersections = getIntersectedCells(grid, pathCoords);
    removeEnteringIntersections(&intersections);

    if (intersections.empty()) return intersectionInfos;

    cvf::Vec3d            startPoint;
    cvf::Vec3d            endPoint;
    size_t                cellIndex;
    cvf::Vec3d            internalCellLengths;

    auto intersection = intersections.cbegin();

    //start cell
    bool foundCell;
    startPoint = pathCoords[0];
    cellIndex = findCellFromCoords(grid, startPoint, &foundCell);
    if (foundCell)
    {
        endPoint = intersection->m_intersectionPoint;
        internalCellLengths = calculateLengthInCell(grid, cellIndex, startPoint, endPoint);
        intersectionInfos.push_back(WellPathCellIntersectionInfo(cellIndex, startPoint, endPoint, internalCellLengths));
    }
    else
    {
        RiaLogging::debug("Path starts outside valid cell");
    }

    //center cells
    startPoint = intersection->m_intersectionPoint;
    cellIndex = intersection->m_hexIndex;

    ++intersection;

    while (intersection != intersections.cend())
    {
        endPoint = intersection->m_intersectionPoint;
        internalCellLengths = calculateLengthInCell(grid, cellIndex, startPoint, endPoint);
        intersectionInfos.push_back(WellPathCellIntersectionInfo(cellIndex, startPoint, endPoint, internalCellLengths));

        startPoint = endPoint;
        cellIndex = intersection->m_hexIndex;
        ++intersection;
    }

    //end cell
    endPoint = pathCoords[pathCoords.size() - 1];
    internalCellLengths = calculateLengthInCell(grid, cellIndex, startPoint, endPoint);
    intersectionInfos.push_back(WellPathCellIntersectionInfo(cellIndex, startPoint, endPoint, internalCellLengths));

    return intersectionInfos;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<HexIntersectionInfo> RigWellPathIntersectionTools::getIntersectedCells(const RigMainGrid* grid, const std::vector<cvf::Vec3d>& coords)
{
    std::vector<HexIntersectionInfo> intersections;
    for (size_t i = 0; i < coords.size() - 1; ++i)
    {
        cvf::BoundingBox bb;
        bb.add(coords[i]);
        bb.add(coords[i + 1]);

        std::vector<size_t> closeCells = findCloseCells(grid, bb);

        std::array<cvf::Vec3d, 8> hexCorners;

        for (size_t closeCell : closeCells)
        {
            const RigCell& cell = grid->globalCellArray()[closeCell];
            if (cell.isInvalid()) continue;

            grid->cellCornerVertices(closeCell, hexCorners.data());

            RigHexIntersectionTools::lineHexCellIntersection(coords[i], coords[i + 1], hexCorners.data(), closeCell, &intersections);
        }
    }

    return intersections;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigWellPathIntersectionTools::calculateLengthInCell(const std::array<cvf::Vec3d, 8>& hexCorners, const cvf::Vec3d& startPoint, const cvf::Vec3d& endPoint)
{
    cvf::Vec3d vec = endPoint - startPoint;
    cvf::Vec3d iAxisDirection;
    cvf::Vec3d jAxisDirection;
    cvf::Vec3d kAxisDirection;

    RigCellGeometryTools::findCellLocalXYZ(hexCorners, iAxisDirection, jAxisDirection, kAxisDirection);

    cvf::Mat3d localCellCoordinateSystem(iAxisDirection.x(), jAxisDirection.x(), kAxisDirection.x(),
                                         iAxisDirection.y(), jAxisDirection.y(), kAxisDirection.y(),
                                         iAxisDirection.z(), jAxisDirection.z(), kAxisDirection.z());

    return vec.getTransformedVector(localCellCoordinateSystem.getInverted());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigWellPathIntersectionTools::calculateLengthInCell(const RigMainGrid* grid, size_t cellIndex, const cvf::Vec3d& startPoint, const cvf::Vec3d& endPoint)
{
    std::array<cvf::Vec3d, 8> hexCorners;
    grid->cellCornerVertices(cellIndex, hexCorners.data());

    return calculateLengthInCell(hexCorners, startPoint, endPoint);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RigWellPathIntersectionTools::findCloseCells(const RigMainGrid* grid, const cvf::BoundingBox& bb)
{
    std::vector<size_t> closeCells;
    grid->findIntersectingCells(bb, &closeCells);
    return closeCells;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigWellPathIntersectionTools::findCellFromCoords(const RigMainGrid* grid, const cvf::Vec3d& coords, bool* foundCell)
{
    cvf::BoundingBox bb;
    bb.add(coords);
    std::vector<size_t> closeCells = findCloseCells(grid, bb);
    std::array<cvf::Vec3d, 8> hexCorners;

    for (size_t closeCell : closeCells)
    {
        const RigCell& cell = grid->globalCellArray()[closeCell];
        if (cell.isInvalid()) continue;

        grid->cellCornerVertices(closeCell, hexCorners.data());

        if (RigHexIntersectionTools::isPointInCell(coords, hexCorners.data()))
        {
            *foundCell = true;
            return closeCell;
        }
    }

    *foundCell = false;
    return 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigWellPathIntersectionTools::removeEnteringIntersections(std::vector<HexIntersectionInfo>* intersections)
{
    for (auto it = intersections->begin(); it != intersections->end();)
    {
        if (it->m_isIntersectionEntering)
        {
            it = intersections->erase(it);
        }
        else
        {
            ++it;
        }
    }
}

