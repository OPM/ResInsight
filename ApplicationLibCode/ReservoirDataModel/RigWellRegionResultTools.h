/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RigMdRange.h"

#include "cvfVector3.h"

#include <QString>

#include <optional>
#include <vector>

class RigEclipseCaseData;
class RigSimWellData;
class RigWellPath;

//==================================================================================================
/// Creates a static binary (1/0) cell result in the GENERATED category. A cell is 1 if it is inside
/// the polygon (any corner inside, XY only), or if it is a perforated cell of a selected simulation
/// well, or if it is a perforated cell of the optional well path.
///
/// Only the perforated parts of wells are used:
///  - Simulation well: the open connection cells at the time step, and the pipe cells between the first
///    and last open connection of each branch.
///  - Well path: the cells intersected by the perforation MD range. No range means no well path cells.
/// A simulation well is selected if at least one of its perforated cells is inside the polygon.
///
/// The perforated well cells are grown by growCellCount layers of lateral (I, J) neighbor cells. If
/// fillFullKRange is true, the grown region covers the full K range of its columns. No growth and no
/// K filling is done when growCellCount is 0. If fillFullKRange is false, the polygon only marks cells
/// within the K range spanned by the well cells (if there are any).
//==================================================================================================
class RigWellRegionResultTools
{
public:
    // Returns false on invalid input (null case, polygon with < 3 points, time step out of range)
    static bool createWellRegionResult( RigEclipseCaseData*              eclipseCaseData,
                                        const std::vector<cvf::Vec3d>&   polygon,
                                        const RigWellPath*               wellPath,
                                        const std::optional<RigMdRange>& wellPathPerforation,
                                        size_t                           timeStepIndex,
                                        const QString&                   resultName,
                                        int                              growCellCount  = 1,
                                        bool                             fillFullKRange = true );

    // Reservoir cell indices of the perforated cells of a simulation well, sorted and unique
    static std::vector<size_t>
        simWellPerforatedCells( RigEclipseCaseData* eclipseCaseData, const RigSimWellData* simWellData, size_t timeStepIndex );

    // Reservoir cell indices of the cells intersected by the well path within the perforation MD range
    static std::vector<size_t>
        wellPathPerforatedCells( RigEclipseCaseData* eclipseCaseData, const RigWellPath* wellPath, const RigMdRange& perforation );
};
