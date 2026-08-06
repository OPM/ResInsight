/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "cvfArray.h"
#include "cvfObject.h"

#include <vector>

class RimIjkIntersection;
class RigMainGrid;
class RivIntersectionHexGridInterface;

namespace cvf
{
class DrawableGeo;
} // namespace cvf

//==================================================================================================
/// Generates the geometry for an intersection following the grid pillars at a fixed i/j/k index.
/// The surface is the union of one cell face of every cell in the index plane, so the triangle
/// vertices are native grid corner nodes.
//==================================================================================================
class RivIjkIntersectionGeometryGenerator : public cvf::Object, public RivIntersectionGeometryGeneratorInterface
{
public:
    RivIjkIntersectionGeometryGenerator( RimIjkIntersection*                    intersection,
                                         const RivIntersectionHexGridInterface* grid,
                                         const RigMainGrid*                     mainGrid );

    ~RivIjkIntersectionGeometryGenerator() override;

    // Generate geometry
    cvf::ref<cvf::DrawableGeo> generateSurface( cvf::UByteArray* visibleCells );
    cvf::ref<cvf::DrawableGeo> createMeshDrawable();

    RimIjkIntersection* intersection() const;

    // GeomGen Interface

    bool isAnyGeometryPresent() const override;

    const std::vector<size_t>&                       triangleToCellIndex() const override;
    const std::vector<RivIntersectionVertexWeights>& triangleVxToCellCornerInterpolationWeights() const override;
    const cvf::Vec3fArray*                           triangleVxes() const override;

private:
    void calculateArrays( cvf::UByteArray* visibleCells );

    cvf::cref<RivIntersectionHexGridInterface> m_hexGrid;
    cvf::cref<RigMainGrid>                     m_mainGrid;

    // Output arrays
    cvf::ref<cvf::Vec3fArray>                 m_triangleVxes;
    cvf::ref<cvf::Vec3fArray>                 m_cellBorderLineVxes;
    std::vector<size_t>                       m_triangleToCellIdxMap;
    std::vector<RivIntersectionVertexWeights> m_triVxToCellCornerWeights;

    RimIjkIntersection* m_intersectionDefinition;
};
