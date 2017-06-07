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
std::vector<WellPathCellIntersectionInfo> RigWellPathIntersectionTools::findCellsIntersectedByPath(const RigEclipseCaseData* caseData, const std::vector<cvf::Vec3d>& coords, bool includeStartCell, bool includeEndCell)
{
    std::vector<WellPathCellIntersectionInfo> intersectionInfo;
    const RigMainGrid* grid = caseData->mainGrid();

    if (coords.size() < 2) return intersectionInfo;

    std::vector<HexIntersectionInfo> intersections = getIntersectedCells(grid, coords);
    removeEnteringIntersections(&intersections);

    if (intersections.empty()) return intersectionInfo;

    cvf::Vec3d            startPoint;
    cvf::Vec3d            endPoint;
    size_t                cellIndex;
    cvf::Vec3d            internalCellLengths;

    auto intersection = intersections.cbegin();

    if (includeStartCell)
    {
        bool foundCell;
        startPoint = coords[0];
        cellIndex = findCellFromCoords(grid, startPoint, &foundCell);
        if (foundCell)
        {
            endPoint = intersection->m_intersectionPoint;
            internalCellLengths = calculateLengthInCell(grid, cellIndex, startPoint, endPoint);
            intersectionInfo.push_back(WellPathCellIntersectionInfo(cellIndex, startPoint, endPoint, internalCellLengths));
        }
        else
        {
            RiaLogging::debug("Path starts outside valid cell");
        }
    }

    startPoint = intersection->m_intersectionPoint;
    cellIndex = intersection->m_hexIndex;

    ++intersection;

    while (intersection != intersections.cend())
    {
        endPoint = intersection->m_intersectionPoint;
        internalCellLengths = calculateLengthInCell(grid, cellIndex, startPoint, endPoint);
        intersectionInfo.push_back(WellPathCellIntersectionInfo(cellIndex, startPoint, endPoint, internalCellLengths));

        startPoint = endPoint;
        cellIndex = intersection->m_hexIndex;
        ++intersection;
    }

    if (includeEndCell)
    {
        endPoint = coords[coords.size() - 1];
        internalCellLengths = calculateLengthInCell(grid, cellIndex, startPoint, endPoint);
        intersectionInfo.push_back(WellPathCellIntersectionInfo(cellIndex, startPoint, endPoint, internalCellLengths));
    }

    return intersectionInfo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<HexIntersectionInfo> RigWellPathIntersectionTools::getIntersectedCells(const RigMainGrid* grid, const std::vector<cvf::Vec3d>& coords)
{
    const std::vector<cvf::Vec3d>& nodeCoords = grid->nodes();
    std::vector<HexIntersectionInfo> intersections;
    for (size_t i = 0; i < coords.size() - 1; ++i)
    {
        cvf::BoundingBox bb;
        bb.add(coords[i]);
        bb.add(coords[i + 1]);

        std::vector<size_t> closeCells = findCloseCells(grid, bb);

        cvf::Vec3d hexCorners[8];

        for (size_t closeCell : closeCells)
        {
            const RigCell& cell = grid->globalCellArray()[closeCell];
            if (cell.isInvalid()) continue;

            setHexCorners(cell, nodeCoords, hexCorners);

            RigHexIntersector::lineHexCellIntersection(coords[i], coords[i + 1], hexCorners, closeCell, &intersections);
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
    std::array<cvf::Vec3d, 8> hexCorners = getCellHexCorners(grid, cellIndex);

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
    const std::vector<cvf::Vec3d>& nodeCoords = grid->nodes();

    cvf::BoundingBox bb;
    bb.add(coords);
    std::vector<size_t> closeCells = findCloseCells(grid, bb);
    cvf::Vec3d hexCorners[8];

    for (size_t closeCell : closeCells)
    {
        const RigCell& cell = grid->globalCellArray()[closeCell];
        if (cell.isInvalid()) continue;

        setHexCorners(cell, nodeCoords, hexCorners);

        if (RigHexIntersector::isPointInCell(coords, hexCorners))
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
std::array<cvf::Vec3d, 8> RigWellPathIntersectionTools::getCellHexCorners(const RigMainGrid* grid, size_t cellIndex)
{
    const std::vector<cvf::Vec3d>& nodeCoords = grid->nodes();
    std::array<cvf::Vec3d, 8> corners;
    const RigCell& cell = grid->globalCellArray()[cellIndex];
    setHexCorners(cell, nodeCoords, corners.data());

    return corners;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigWellPathIntersectionTools::setHexCorners(const RigCell& cell, const std::vector<cvf::Vec3d>& nodeCoords, cvf::Vec3d* hexCorners)
{
    const caf::SizeTArray8& cornerIndices = cell.cornerIndices();

    hexCorners[0] = nodeCoords[cornerIndices[0]];
    hexCorners[1] = nodeCoords[cornerIndices[1]];
    hexCorners[2] = nodeCoords[cornerIndices[2]];
    hexCorners[3] = nodeCoords[cornerIndices[3]];
    hexCorners[4] = nodeCoords[cornerIndices[4]];
    hexCorners[5] = nodeCoords[cornerIndices[5]];
    hexCorners[6] = nodeCoords[cornerIndices[6]];
    hexCorners[7] = nodeCoords[cornerIndices[7]];
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

