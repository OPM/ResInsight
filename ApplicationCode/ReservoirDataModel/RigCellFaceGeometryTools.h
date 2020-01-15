/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "cvfCollection.h"
#include "cvfStructGrid.h"
#include "cvfVector3.h"

#include <vector>

class RigCell;
class RigMainGrid;
class RigConnection;

//==================================================================================================
///
//==================================================================================================
class RigCellFaceGeometryTools
{
public:
    static cvf::StructGridInterface::FaceType calculateCellFaceOverlap( const RigCell&           c1,
                                                                        const RigCell&           c2,
                                                                        const RigMainGrid&       mainGrid,
                                                                        std::vector<size_t>*     connectionPolygon,
                                                                        std::vector<cvf::Vec3d>* connectionIntersections );

    static std::vector<RigConnection> computeOtherNncs( const RigMainGrid*                mainGrid,
                                                        const std::vector<RigConnection>& nativeConnections );

    static std::vector<cvf::Vec3d> extractPolygon( const std::vector<cvf::Vec3d>& nativeNodes,
                                                   const std::vector<size_t>&     connectionPolygon,
                                                   const std::vector<cvf::Vec3d>& connectionIntersections );
};
