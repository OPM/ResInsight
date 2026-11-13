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

#include "cvfVector3.h"

#include <QString>

#include <vector>

class RigEclipseCaseData;
class RigWellPath;

//==================================================================================================
/// Creates a static binary (1/0) cell result in the GENERATED category. A cell is 1 if it is
/// inside the polygon (any corner inside, XY only), or if it is a connection cell or intersected by
/// the pipe of a simulation well where at least one connection cell or pipe cell is inside the polygon, or if it is
/// intersected by the optional well path. The well cells are grown by growCellCount layers of neighbor cells.
//==================================================================================================
class RigWellRegionResultTools
{
public:
    // Returns false on invalid input (null case, polygon with < 3 points, time step out of range)
    static bool createWellRegionResult( RigEclipseCaseData*            eclipseCaseData,
                                        const std::vector<cvf::Vec3d>& polygon,
                                        const RigWellPath*             wellPath,
                                        size_t                         timeStepIndex,
                                        const QString&                 resultName,
                                        int                            growCellCount = 1 );
};
