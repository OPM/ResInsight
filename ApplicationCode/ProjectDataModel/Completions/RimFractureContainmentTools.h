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

#pragma once

#include "cvfStructGrid.h"

#include <set>
#include <vector>

class RigMainGrid;
class RigActiveCellInfo;
class RimFracture;
class RimEclipseCase;

class RimFractureContainmentTools
{
public:
    static std::set<size_t> fracturedCellsTruncatedByFaults(const RimEclipseCase* eclipseCase, RimFracture* fracture);

private:
    // TODO: Try to change RimStimPlanFractureTemplate::fractureTriangleGeometry to be able to use const RimFracture
    static std::set<size_t>
        getCellsIntersectingFracturePlane(const RigMainGrid* mainGrid, const RigActiveCellInfo* activeCellInfo, RimFracture* fracture);

    static void appendNeighborCellForFace(const std::set<size_t>&            allFracturedCells,
                                          const RigMainGrid*                 mainGrid,
                                          size_t                             currentCell,
                                          cvf::StructGridInterface::FaceType face,
                                          std::set<size_t>&                  connectedCells);

    static void checkFaultAndAppendNeighborCell(const std::set<size_t>&            allFracturedCells,
                                                const RigMainGrid*                 mainGrid,
                                                size_t                             currentCell,
                                                cvf::StructGridInterface::FaceType face,
                                                std::set<size_t>&                  connectedCells);

    static void appendNeighborCells(const std::set<size_t>& allFracturedCells,
                                    const RigMainGrid*      mainGrid,
                                    size_t                  currentCell,
                                    std::set<size_t>&       connectedCells);
};
