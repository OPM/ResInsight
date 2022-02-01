/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 - Equinor ASA
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

#include "RivIntersectionVertexWeights.h"

#include "cvfArray.h"

#include <vector>

class RivIntersectionGeometryGeneratorInterface
{
public:
    virtual ~RivIntersectionGeometryGeneratorInterface()                                  = default;
    virtual bool                                             isAnyGeometryPresent() const = 0;
    virtual const std::vector<size_t>&                       triangleToCellIndex() const  = 0;
    virtual const std::vector<RivIntersectionVertexWeights>& triangleVxToCellCornerInterpolationWeights() const = 0;
    virtual const cvf::Vec3fArray*                           triangleVxes() const                               = 0;
    virtual const cvf::Vec3fArray*                           cellMeshVxes() const { return nullptr; };
    virtual const cvf::Vec3fArray*                           faultMeshVxes() const { return nullptr; };
};
