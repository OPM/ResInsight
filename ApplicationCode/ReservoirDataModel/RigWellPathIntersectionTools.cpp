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
    WellPathCellDirection direction;

    auto& intersection = intersections.cbegin();

    if (includeStartCell)
    {
        startPoint = coords[0];
        endPoint = intersection->m_intersectionPoint;
        cellIndex = findCellFromCoords(grid, startPoint);
        direction = calculateDirectionInCell(grid, cellIndex, startPoint, endPoint);
        intersectionInfo.push_back(WellPathCellIntersectionInfo(cellIndex, direction, startPoint, endPoint));
    }

    startPoint = intersection->m_intersectionPoint;
    cellIndex = intersection->m_hexIndex;

    ++intersection;

    while (intersection != intersections.cend())
    {
        endPoint = intersection->m_intersectionPoint;
        direction = calculateDirectionInCell(grid, cellIndex, startPoint, endPoint);
        intersectionInfo.push_back(WellPathCellIntersectionInfo(cellIndex, direction, startPoint, endPoint));

        startPoint = endPoint;
        cellIndex = intersection->m_hexIndex;
        ++intersection;
    }

    if (includeEndCell)
    {
        endPoint = coords[coords.size() - 1];
        direction = calculateDirectionInCell(grid, cellIndex, startPoint, endPoint);
        intersectionInfo.push_back(WellPathCellIntersectionInfo(cellIndex, direction, startPoint, endPoint));
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
/// Calculates direction vectors for each axis in the cell.
//--------------------------------------------------------------------------------------------------
void RigWellPathIntersectionTools::calculateCellMainAxisVectors(const std::array<cvf::Vec3d, 8>& hexCorners, cvf::Vec3d* iAxisDirection, cvf::Vec3d* jAxisDirection, cvf::Vec3d* kAxisDirection)
{
    for (auto vec : hexCorners)
    {
        RiaLogging::debug(QString("%1, %2, %3").arg(vec.x()).arg(vec.y()).arg(vec.z()));
    }
    *iAxisDirection = calculateCellMainAxisVector(hexCorners, cvf::StructGridInterface::NEG_I, cvf::StructGridInterface::POS_I);
    *jAxisDirection = calculateCellMainAxisVector(hexCorners, cvf::StructGridInterface::NEG_J, cvf::StructGridInterface::POS_J);
    *kAxisDirection = calculateCellMainAxisVector(hexCorners, cvf::StructGridInterface::NEG_K, cvf::StructGridInterface::POS_K);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigWellPathIntersectionTools::calculateCellMainAxisVector(const std::array<cvf::Vec3d, 8>& hexCorners,
                                                                        cvf::StructGridInterface::FaceType negativeFace,
                                                                        cvf::StructGridInterface::FaceType positiveFace)
{
    cvf::ubyte negativeFaceVertexIndices[4];
    cvf::StructGridInterface::cellFaceVertexIndices(negativeFace, negativeFaceVertexIndices);
    cvf::Vec3d negativeFaceCenter = cvf::GeometryTools::computeFaceCenter(hexCorners[negativeFaceVertexIndices[0]],
                                                                          hexCorners[negativeFaceVertexIndices[1]],
                                                                          hexCorners[negativeFaceVertexIndices[2]],
                                                                          hexCorners[negativeFaceVertexIndices[3]]);

    cvf::ubyte positiveFaceVertexIndices[4];
    cvf::StructGridInterface::cellFaceVertexIndices(positiveFace, positiveFaceVertexIndices);
    cvf::Vec3d positiveFaceCenter = cvf::GeometryTools::computeFaceCenter(hexCorners[positiveFaceVertexIndices[0]],
                                                                          hexCorners[positiveFaceVertexIndices[1]],
                                                                          hexCorners[positiveFaceVertexIndices[2]],
                                                                          hexCorners[positiveFaceVertexIndices[3]]);

    return positiveFaceCenter - negativeFaceCenter;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
WellPathCellDirection RigWellPathIntersectionTools::calculateDirectionInCell(const std::array<cvf::Vec3d, 8>& hexCorners, const cvf::Vec3d& startPoint, const cvf::Vec3d& endPoint)
{
    cvf::Vec3d vec = endPoint - startPoint;

    cvf::Vec3d iAxisVector;
    cvf::Vec3d jAxisVector;
    cvf::Vec3d kAxisVector;
    calculateCellMainAxisVectors(hexCorners, &iAxisVector, &jAxisVector, &kAxisVector);

    cvf::Vec3d iAxisDirection = iAxisVector.getNormalized();
    cvf::Vec3d jAxisDirection = jAxisVector.getNormalized();
    cvf::Vec3d kAxisDirection = kAxisVector.getNormalized();

    cvf::Mat3d localCellCoordinateSystem(iAxisDirection.x(), jAxisDirection.x(), kAxisDirection.x(),
                                         iAxisDirection.y(), jAxisDirection.y(), kAxisDirection.y(),
                                         iAxisDirection.z(), jAxisDirection.z(), kAxisDirection.z());

    cvf::Vec3d localCellCoordinateVec = vec.getTransformedVector(localCellCoordinateSystem.getInverted());

    double iLengthFraction = abs(localCellCoordinateVec.x() / iAxisVector.length());
    double jLengthFraction = abs(localCellCoordinateVec.y() / jAxisVector.length());
    double kLengthFraction = abs(localCellCoordinateVec.z() / kAxisVector.length());

    if (iLengthFraction > jLengthFraction && iLengthFraction > kLengthFraction)
    {
        WellPathCellDirection direction = POS_I;
        if (localCellCoordinateVec.x() < 0)
        {
            direction = NEG_I;
        }
        return direction;
    }
    else if (jLengthFraction > iLengthFraction && jLengthFraction > kLengthFraction)
    {
        WellPathCellDirection direction = POS_J;
        if (localCellCoordinateVec.y() < 0)
        {
            direction = NEG_J;
        }
        return direction;
    }
    else
    {
        WellPathCellDirection direction = POS_K;
        if (localCellCoordinateVec.z() < 0)
        {
            direction = NEG_K;
        }
        return direction;
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
WellPathCellDirection RigWellPathIntersectionTools::calculateDirectionInCell(const RigMainGrid* grid, size_t cellIndex, const cvf::Vec3d& startPoint, const cvf::Vec3d& endPoint)
{
    std::array<cvf::Vec3d, 8> hexCorners = getCellHexCorners(grid, cellIndex);

    return calculateDirectionInCell(hexCorners, startPoint, endPoint);
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
size_t RigWellPathIntersectionTools::findCellFromCoords(const RigMainGrid* grid, const cvf::Vec3d& coords)
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

        if (RigHexIntersector::isPointInCell(coords, hexCorners, closeCell))
        {
            return closeCell;
        }
    }

    // Coordinate is outside any cells?
    CVF_ASSERT(false);
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

