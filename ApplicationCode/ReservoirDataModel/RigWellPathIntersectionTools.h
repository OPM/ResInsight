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

#pragma once

#include "RigCell.h"

#include "RigWellLogExtractionTools.h"

#include "cvfVector3.h"

#include <array>

class RigWellPath;
class RigMainGrid;
class RigEclipseCaseData;

//==================================================================================================
/// 
//==================================================================================================
enum WellPathCellDirection {
    POS_I,
    NEG_I,
    POS_J,
    NEG_J,
    POS_K,
    NEG_K
};

//==================================================================================================
/// 
//==================================================================================================
struct WellPathCellIntersectionInfo {
    WellPathCellIntersectionInfo(size_t cellIndex, WellPathCellDirection direction, cvf::Vec3d startPoint, cvf::Vec3d endPoint)
        : cellIndex(cellIndex),
          direction(direction),
          startPoint(startPoint),
          endPoint(endPoint)
    {}

    size_t                cellIndex;
    WellPathCellDirection direction;
    cvf::Vec3d            startPoint;
    cvf::Vec3d            endPoint;
};

//==================================================================================================
/// 
//==================================================================================================
class RigWellPathIntersectionTools
{
public:
    static std::vector<WellPathCellIntersectionInfo>   findCellsIntersectedByPath(const RigEclipseCaseData* caseData, const std::vector<cvf::Vec3d>& coords, bool includeStartCell = true, bool includeEndCell = true);

    static std::vector<HexIntersectionInfo>            getIntersectedCells(const RigMainGrid* grid, const std::vector<cvf::Vec3d>& coords);

    // Calculate direction
    static void                                        calculateCellMainAxisVectors(const std::array<cvf::Vec3d, 8>& hexCorners, cvf::Vec3d* iAxisDirection, cvf::Vec3d* jAxisDirection, cvf::Vec3d* kAxisDirection);
    static cvf::Vec3d                                  calculateCellMainAxisVector(const std::array<cvf::Vec3d, 8>& hexCorners, cvf::StructGridInterface::FaceType startFace, cvf::StructGridInterface::FaceType endFace);
    static WellPathCellDirection                       calculateDirectionInCell(const std::array<cvf::Vec3d, 8>& hexCorners, const cvf::Vec3d& startPoint, const cvf::Vec3d& endPoint);
    static WellPathCellDirection                       calculateDirectionInCell(const RigMainGrid* grid, size_t cellIndex, const cvf::Vec3d& startPoint, const cvf::Vec3d& endPoint);

    static std::vector<size_t>                         findCloseCells(const RigMainGrid* grid, const cvf::BoundingBox& bb);
    static size_t                                      findCellFromCoords(const RigMainGrid* caseData, const cvf::Vec3d& coords, bool* foundCell);

    static std::array<cvf::Vec3d, 8>                   getCellHexCorners(const RigMainGrid* grid, size_t cellIndex);
    static void                                        setHexCorners(const RigCell& cell, const std::vector<cvf::Vec3d>& nodeCoords, cvf::Vec3d* hexCorners);

private:
    static void                                        removeEnteringIntersections(std::vector<HexIntersectionInfo>* intersections);
};
