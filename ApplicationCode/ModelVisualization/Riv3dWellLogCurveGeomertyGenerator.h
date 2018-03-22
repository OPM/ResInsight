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
class RimWellPath;

class Riv3dWellLogCurveGeometryGenerator : public cvf::Object
{
public:
    Riv3dWellLogCurveGeometryGenerator(RimWellPath* wellPath);

    cvf::ref<cvf::DrawableGeo> createCurveLine(const caf::DisplayCoordTransform* displayCoordTransform,
                                               const cvf::BoundingBox&           wellPathClipBoundingBox,
                                               const std::vector<double>&        resultValues,
                                               const std::vector<double>&        resultMds,
                                               double                            planeAngle,
                                               double                            planeOffsetFromWellPathCenter,
                                               double                            planeWidth) const;

    cvf::ref<cvf::DrawableGeo> createGrid(const caf::DisplayCoordTransform*  displayCoordTransform,
                                          const cvf::BoundingBox&            wellPathClipBoundingBox,
                                          double                             planeAngle,
                                          double                             planeOffsetFromWellPathCenter,
                                          double                             planeWidth,
                                          double                             gridIntervalSize) const;
private:
    enum VertexOrganization
    {
        LINE_SEGMENTS,
        POLYLINE
    };

private:
    void createCurveVerticesAndIndices(const std::vector<double>&        resultValues,
                                       const std::vector<double>&        resultMds,
                                       double                            planeAngle,
                                       double                            planeOffsetFromWellPathCenter,
                                       double                            planeWidth,
                                       const caf::DisplayCoordTransform* displayCoordTransform,
                                       const cvf::BoundingBox&           wellPathClipBoundingBox,
                                       std::vector<cvf::Vec3f>*          vertices,
                                       std::vector<cvf::uint>*           indices) const;

    std::vector<cvf::Vec3d> calculateLineSegmentNormals(double                         angle,
                                                        const std::vector<cvf::Vec3d>& vertices,
                                                        VertexOrganization             organization) const;

    const RigWellPath* wellPathGeometry() const;

    void calculatePairsOfClosestSamplingPointsAlongWellPath(std::vector<cvf::Vec3d>* closestWellPathPoints,
                                                            std::vector<cvf::Vec3d>& points) const;

private:
    caf::PdmPointer<RimWellPath> m_wellPath;
};
