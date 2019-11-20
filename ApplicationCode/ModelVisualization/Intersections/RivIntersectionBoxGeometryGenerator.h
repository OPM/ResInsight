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

#include "RivHexGridIntersectionTools.h"

#include "cafPdmPointer.h"

#include "cvfArray.h"

#include "cvfObject.h"
#include "cvfVector3.h"

#include <vector>

class RimIntersectionBox;

namespace cvf
{
class ScalarMapper;
class DrawableGeo;
} // namespace cvf

class RivIntersectionGeometryGeneratorIF
{
public:
    virtual bool                                             isAnyGeometryPresent() const                       = 0;
    virtual const std::vector<size_t>&                       triangleToCellIndex() const                        = 0;
    virtual const std::vector<RivIntersectionVertexWeights>& triangleVxToCellCornerInterpolationWeights() const = 0;
    virtual const cvf::Vec3fArray*                           triangleVxes() const                               = 0;
};

class RivIntersectionBoxGeometryGenerator : public cvf::Object, public RivIntersectionGeometryGeneratorIF
{
public:
    RivIntersectionBoxGeometryGenerator( RimIntersectionBox*                    intersectionBox,
                                         const RivIntersectionHexGridInterface* grid );

    ~RivIntersectionBoxGeometryGenerator() override;

    bool isAnyGeometryPresent() const override;
    // Mapping between cells and geometry
    const std::vector<size_t>&                       triangleToCellIndex() const override;
    const std::vector<RivIntersectionVertexWeights>& triangleVxToCellCornerInterpolationWeights() const override;
    const cvf::Vec3fArray*                           triangleVxes() const override;

    // Generate geometry
    cvf::ref<cvf::DrawableGeo> generateSurface();
    cvf::ref<cvf::DrawableGeo> createMeshDrawable();


    RimIntersectionBox* intersectionBox() const;

private:
    void calculateArrays();

    cvf::cref<RivIntersectionHexGridInterface> m_hexGrid;

    // Output arrays
    cvf::ref<cvf::Vec3fArray>                 m_triangleVxes;
    cvf::ref<cvf::Vec3fArray>                 m_cellBorderLineVxes;
    std::vector<size_t>                       m_triangleToCellIdxMap;
    std::vector<RivIntersectionVertexWeights> m_triVxToCellCornerWeights;

    RimIntersectionBox* m_intersectionBoxDefinition;
};
