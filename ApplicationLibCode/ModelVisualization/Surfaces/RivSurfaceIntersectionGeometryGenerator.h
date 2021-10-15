/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "cafPdmPointer.h"

#include "RivIntersectionGeometryGeneratorInterface.h"

#include "cvfArray.h"
#include "cvfBoundingBox.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include <QString>

#include <vector>

class RigMainGrid;
class RigActiveCellInfo;
class RigResultAccessor;
class RigSurface;

class RimSurfaceInView;
class RivIntersectionHexGridInterface;
class RivIntersectionVertexWeights;

namespace cvf
{
class ScalarMapper;
class DrawableGeo;
} // namespace cvf

class RivSurfaceIntersectionGeometryGenerator : public cvf::Object, public RivIntersectionGeometryGeneratorInterface
{
public:
    RivSurfaceIntersectionGeometryGenerator( RimSurfaceInView* surfInView, const RivIntersectionHexGridInterface* grid );

    ~RivSurfaceIntersectionGeometryGenerator() override;

    // Generate geometry
    cvf::ref<cvf::DrawableGeo> generateSurface();
    cvf::ref<cvf::DrawableGeo> createMeshDrawable();
    cvf::ref<cvf::DrawableGeo> createFaultMeshDrawable();

    const std::vector<std::pair<QString, cvf::Vec3d>>& faultMeshLabelAndAnchorPositions();

    RimSurfaceInView* intersection() const;

    // GeomGen Interface
    bool isAnyGeometryPresent() const override;

    const std::vector<size_t>&                       triangleToCellIndex() const override;
    const std::vector<RivIntersectionVertexWeights>& triangleVxToCellCornerInterpolationWeights() const override;
    const cvf::Vec3fArray*                           triangleVxes() const override;

private:
    void calculateArrays();

    RimSurfaceInView*    m_surfaceInView;
    cvf::ref<RigSurface> m_usedSurfaceData; // Store the reference to the old data, to know when new data has arrived.

    cvf::cref<RivIntersectionHexGridInterface> m_hexGrid;

    // Output arrays
    cvf::ref<cvf::Vec3fArray>                 m_triangleVxes;
    cvf::ref<cvf::Vec3fArray>                 m_cellBorderLineVxes;
    cvf::ref<cvf::Vec3fArray>                 m_faultCellBorderLineVxes;
    std::vector<size_t>                       m_triangleToCellIdxMap;
    std::vector<RivIntersectionVertexWeights> m_triVxToCellCornerWeights;

    std::vector<std::pair<QString, cvf::Vec3d>> m_faultMeshLabelAndAnchorPositions;
};
