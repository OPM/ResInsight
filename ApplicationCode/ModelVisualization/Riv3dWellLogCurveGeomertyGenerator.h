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
    typedef std::pair<cvf::Vec3d, double> PointValuePair;
    Riv3dWellLogCurveGeometryGenerator(RimWellPath* wellPath);

    void createCurveDrawables(const caf::DisplayCoordTransform* displayCoordTransform,
                              const cvf::BoundingBox&           wellPathClipBoundingBox,
                              const std::vector<double>&        resultValues,
                              const std::vector<double>&        resultMds,
                              double                            planeAngle,
                              double                            planeOffsetFromWellPathCenter,
                              double                            planeWidth,
                              double                            minResultValue,
                              double                            maxResultValue);

    const RigWellPath* wellPathGeometry() const;

    cvf::ref<cvf::DrawableGeo> curveDrawable();

    bool findClosestPointOnCurve(const cvf::Vec3d& globalIntersection,
                                 cvf::Vec3d*       closestPoint,
                                 double*           measuredDepthAtPoint,
                                 double*           valueAtClosestPoint) const;

private:
    caf::PdmPointer<RimWellPath> m_wellPath;
    cvf::ref<cvf::DrawableGeo>   m_curveDrawable;
    std::vector<cvf::Vec3f>      m_curveVertices;
    std::vector<double>          m_curveMeasuredDepths;
    std::vector<double>          m_curveValues;
    double                       m_planeWidth;
};
