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

#include <vector>

#include <QString>

class RigMainGrid;
class RigActiveCellInfo;
class RigResultAccessor;
class RivIntersectionHexGridInterface;
class RimSurface;

namespace cvf
{
class ScalarMapper;
class DrawableGeo;
} // namespace cvf

class RivPolylineIntersectionGeometryGenerator : public cvf::Object, public RivIntersectionGeometryGeneratorInterface
{
public:
    RivPolylineIntersectionGeometryGenerator( std::vector<cvf::Vec3d>& polyline, RivIntersectionHexGridInterface* grid );
    ~RivPolylineIntersectionGeometryGenerator() override;

    void                       generateIntersectionGeometry( cvf::UByteArray* visibleCells );
    const cvf::Vec3fArray*     polygonVxes() const;
    const std::vector<size_t>& vertiesPerPolygon() const;
    const std::vector<size_t>& polygonToCellIndex() const;

    // GeomGen Interface
    bool                                             isAnyGeometryPresent() const override;
    const std::vector<size_t>&                       triangleToCellIndex() const override;
    const std::vector<RivIntersectionVertexWeights>& triangleVxToCellCornerInterpolationWeights() const override;
    const cvf::Vec3fArray*                           triangleVxes() const override;

private:
    void                       calculateArrays( cvf::UByteArray* visibleCells );
    static std::vector<size_t> createPolylineSegmentCellCandidates( const RivIntersectionHexGridInterface& hexGrid,
                                                                    const cvf::Vec3d&                      startPoint,
                                                                    const cvf::Vec3d&                      endPoint,
                                                                    const cvf::Vec3d&                      heightVector,
                                                                    const double                           topDepth,
                                                                    const double                           bottomDepth );

private:
    cvf::ref<RivIntersectionHexGridInterface> m_hexGrid;
    const std::vector<cvf::Vec3d>             m_polyline;

    // Output arrays
    cvf::ref<cvf::Vec3fArray> m_triangleVxes;
    std::vector<size_t>       m_triangleToCellIdxMap;

    std::vector<size_t>       m_polygonToCellIdxMap;
    cvf::ref<cvf::Vec3fArray> m_polygonVertices;
    std::vector<size_t>       m_verticesPerPolygon;
};
