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

#include "RigFault.h"
#include "RigNncConnection.h"

#include "cvfCollection.h"
#include "cvfStructGrid.h"
#include "cvfVector3.h"

#include <deque>
#include <set>
#include <utility>
#include <vector>

class RigActiveCellInfo;
class RigCell;
class RigMainGrid;

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

    static RigConnectionContainer computeOtherNncs( const RigMainGrid*            mainGrid,
                                                    const RigConnectionContainer& nativeConnections,
                                                    const RigActiveCellInfo*      activeCellInfo,
                                                    bool                          includeInactiveCells );

    static void                    extractConnectionsForFace( const RigFault::FaultFace&                     face,
                                                              const RigMainGrid*                             mainGrid,
                                                              const std::set<std::pair<unsigned, unsigned>>& nativeCellPairs,
                                                              RigConnectionContainer&                        connections );
    static std::vector<cvf::Vec3f> extractPolygon( const std::vector<cvf::Vec3d>& nativeNodes,
                                                   const std::vector<size_t>&     connectionPolygon,
                                                   const std::vector<cvf::Vec3d>& connectionIntersections );
};
