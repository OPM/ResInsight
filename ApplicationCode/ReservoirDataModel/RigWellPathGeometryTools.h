/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include <vector>

#include "cvfBase.h"
#include "cvfVector3.h"

class RigWellPath;

//==================================================================================================
/// 
//==================================================================================================
class RigWellPathGeometryTools
{
public:
    enum VertexOrganization
    {
        LINE_SEGMENTS,
        POLYLINE
    };

public:
    static std::vector<cvf::Vec3d> calculateLineSegmentNormals(const std::vector<cvf::Vec3d>& vertices,
                                                               double                         angle);
private:
    static std::vector<cvf::Vec3d> interpolateUndefinedNormals(const cvf::Vec3d& planeNormal,
                                                               const std::vector<cvf::Vec3d>& normals,
                                                               const std::vector<cvf::Vec3d>& vertices);
    static cvf::Vec3d estimateDominantDirectionInXYPlane(const std::vector<cvf::Vec3d>& vertices);
};
