/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "RivIntersectionGeometryGeneratorInterface.h"

#include "cafPdmPointer.h"

#include "cvfArray.h"

#include "cvfObject.h"
#include "cvfVector3.h"

#include <vector>

class RimBoxIntersection;
class RivIntersectionHexGridInterface;

namespace cvf
{
class ScalarMapper;
class DrawableGeo;
} // namespace cvf

class RivBoxIntersectionGeometryGenerator : public cvf::Object, public RivIntersectionGeometryGeneratorInterface
{
public:
    RivBoxIntersectionGeometryGenerator( RimBoxIntersection* intersectionBox, const RivIntersectionHexGridInterface* grid );

    ~RivBoxIntersectionGeometryGenerator() override;

    // Generate geometry
    cvf::ref<cvf::DrawableGeo> generateSurface();
    cvf::ref<cvf::DrawableGeo> createMeshDrawable();

    RimBoxIntersection* intersectionBox() const;

    // GeomGen Interface

    bool isAnyGeometryPresent() const override;

    const std::vector<size_t>&                       triangleToCellIndex() const override;
    const std::vector<RivIntersectionVertexWeights>& triangleVxToCellCornerInterpolationWeights() const override;
    const cvf::Vec3fArray*                           triangleVxes() const override;

private:
    void calculateArrays();

    cvf::cref<RivIntersectionHexGridInterface> m_hexGrid;

    // Output arrays
    cvf::ref<cvf::Vec3fArray>                 m_triangleVxes;
    cvf::ref<cvf::Vec3fArray>                 m_cellBorderLineVxes;
    std::vector<size_t>                       m_triangleToCellIdxMap;
    std::vector<RivIntersectionVertexWeights> m_triVxToCellCornerWeights;

    RimBoxIntersection* m_intersectionBoxDefinition;
};
