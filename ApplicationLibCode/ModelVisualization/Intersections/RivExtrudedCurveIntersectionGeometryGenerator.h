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

#include "RivHexGridIntersectionTools.h"

#include "cvfArray.h"

#include "cvfBoundingBox.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include <vector>

#include <QString>

class RigMainGrid;
class RigActiveCellInfo;
class RigResultAccessor;
class RimExtrudedCurveIntersection;
class RivIntersectionHexGridInterface;
class RivIntersectionVertexWeights;
class RimSurface;

namespace cvf
{
class ScalarMapper;
class DrawableGeo;
} // namespace cvf

class RivExtrudedCurveIntersectionGeometryGenerator : public cvf::Object, public RivIntersectionGeometryGeneratorIF
{
public:
    RivExtrudedCurveIntersectionGeometryGenerator( RimExtrudedCurveIntersection*          intersection,
                                                   std::vector<std::vector<cvf::Vec3d>>&  polylines,
                                                   const cvf::Vec3d&                      extrusionDirection,
                                                   const RivIntersectionHexGridInterface* grid,
                                                   bool                                   isFlattened,
                                                   const cvf::Vec3d&                      flattenedPolylineStartPoint );

    ~RivExtrudedCurveIntersectionGeometryGenerator() override;

    // Generate geometry
    cvf::ref<cvf::DrawableGeo> generateSurface();
    cvf::ref<cvf::DrawableGeo> createMeshDrawable();
    cvf::ref<cvf::DrawableGeo> createFaultMeshDrawable();

    cvf::ref<cvf::DrawableGeo> createLineAlongPolylineDrawable();
    cvf::ref<cvf::DrawableGeo> createLineAlongExtrusionLineDrawable( const std::vector<cvf::Vec3d>& extrusionLine );
    cvf::ref<cvf::DrawableGeo> createPointsFromPolylineDrawable();
    cvf::ref<cvf::DrawableGeo> createPointsFromExtrusionLineDrawable( const std::vector<cvf::Vec3d>& extrusionLine );

    const std::vector<std::pair<QString, cvf::Vec3d>>& faultMeshLabelAndAnchorPositions();

    cvf::Mat4d unflattenTransformMatrix( const cvf::Vec3d& intersectionPointFlat ) const;

    // GeomGen Interface
    bool isAnyGeometryPresent() const override;

    std::map<RimSurface*, std::vector<cvf::Vec3d>> transformedSurfaceIntersectionPolylines() const;

    const std::vector<size_t>&                       triangleToCellIndex() const override;
    const std::vector<RivIntersectionVertexWeights>& triangleVxToCellCornerInterpolationWeights() const override;
    const cvf::Vec3fArray*                           triangleVxes() const override;

private:
    void calculateArrays();
    void calculateLineSegementTransforms();
    void calculateTransformedPolyline();
    void calculateSurfaceIntersectionPoints();

    RimExtrudedCurveIntersection* intersection() const;

    const std::vector<std::vector<cvf::Vec3d>>& flattenedOrOffsettedPolyLines();
    cvf::Vec3d transformPointByPolylineSegmentIndex( const cvf::Vec3d& domainCoord, size_t lineIndex, size_t segmentIndex );

    static std::vector<std::pair<cvf::Vec3d, size_t>> computeResampledPolyline( const std::vector<cvf::Vec3d>& polyline,
                                                                                double resamplingDistance );

private:
    RimExtrudedCurveIntersection*              m_intersection;
    cvf::cref<RivIntersectionHexGridInterface> m_hexGrid;
    const std::vector<std::vector<cvf::Vec3d>> m_polylines;
    cvf::Vec3d                                 m_extrusionDirection;
    bool                                       m_isFlattened;
    cvf::Vec3d                                 m_flattenedPolylineStartPoint;

    // Output arrays
    cvf::ref<cvf::Vec3fArray>                 m_triangleVxes;
    cvf::ref<cvf::Vec3fArray>                 m_cellBorderLineVxes;
    cvf::ref<cvf::Vec3fArray>                 m_faultCellBorderLineVxes;
    std::vector<size_t>                       m_triangleToCellIdxMap;
    std::vector<RivIntersectionVertexWeights> m_triVxToCellCornerWeights;
    std::vector<std::vector<cvf::Vec3d>>      m_transformedPolyLines;
    std::vector<std::vector<cvf::Mat4d>>      m_lineSegmentTransforms;

    std::vector<std::pair<QString, cvf::Vec3d>> m_faultMeshLabelAndAnchorPositions;

    std::map<RimSurface*, std::vector<cvf::Vec3d>> m_transformedSurfaceIntersectionPolylines;
};
