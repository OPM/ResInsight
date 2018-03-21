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

#include "cvfBase.h"
#include "cvfDrawableGeo.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include "cafPdmPointer.h"

#include "Rim3dWellLogCurve.h"

#include <vector>

namespace caf
{
class DisplayCoordTransform;
}

namespace cvf
{
class BoundingBox;
}

class RigWellPath;
class RimGridView;
class RimWellPath;

class Riv3dWellLogCurveGeometryGenerator : public cvf::Object
{
public:
    Riv3dWellLogCurveGeometryGenerator(RimWellPath* wellPath, RimGridView* gridView);

    cvf::ref<cvf::DrawableGeo> createCurveLine(const caf::DisplayCoordTransform* displayCoordTransform,
                                               const cvf::BoundingBox&           wellPathClipBoundingBox,
                                               const Rim3dWellLogCurve*          rim3dWellLogCurve) const;

    cvf::ref<cvf::DrawableGeo> createGrid(const caf::DisplayCoordTransform*  displayCoordTransform,
                                          const cvf::BoundingBox&            wellPathClipBoundingBox,
                                          const Rim3dWellLogCurve::DrawPlane drawPlane,
                                          double                             gridIntervalSize) const;

private:
    enum VertexOrganization
    {
        LINE_SEGMENTS,
        POLYLINE
    };

private:
    void createCurveVerticesAndIndices(const Rim3dWellLogCurve*          rim3dWellLogCurve,
                                       const caf::DisplayCoordTransform* displayCoordTransform,
                                       const cvf::BoundingBox&           wellPathClipBoundingBox,
                                       std::vector<cvf::Vec3f>*          vertices,
                                       std::vector<cvf::uint>*           indices) const;

    std::vector<cvf::Vec3d> calculateLineSegmentNormals(Rim3dWellLogCurve::DrawPlane   drawPlane,
                                                        const std::vector<cvf::Vec3d>& vertices,
                                                        VertexOrganization             organization) const;

    double wellPathCenterToPlotStartOffset() const;
    double gridWidth() const;

    const RigWellPath* wellPathGeometry() const;

    void calculatePairsOfClosestPointsAlongWellPath(std::vector<cvf::Vec3d>* closestWellPathPoints,
                                                    std::vector<cvf::Vec3d>& points) const;

private:
    caf::PdmPointer<RimWellPath> m_wellPath;
    caf::PdmPointer<RimGridView> m_gridView;
};
