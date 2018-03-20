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

class RigWellPath;

class Riv3dWellLogCurveGeometryGenerator : public cvf::Object
{
public:
    Riv3dWellLogCurveGeometryGenerator(RigWellPath* wellPathGeometry)
        : m_wellPathGeometry(wellPathGeometry){};

    cvf::ref<cvf::DrawableGeo> createCurveLine(const caf::DisplayCoordTransform* displayCoordTransform,
                                               const Rim3dWellLogCurve*          rim3dWellLogCurve) const;
    cvf::ref<cvf::DrawableGeo> createGrid(const caf::DisplayCoordTransform* displayCoordTransform,
                                          const Rim3dWellLogCurve*          rim3dWellLogCurve,
                                          double                            gridIntervalSize) const;

private:
    void createCurveVerticesAndIndices(const Rim3dWellLogCurve*          rim3dWellLogCurve,
                                       const caf::DisplayCoordTransform* displayCoordTransform,
                                       std::vector<cvf::Vec3f>*          vertices,
                                       std::vector<cvf::uint>*           indices) const;

    static std::vector<cvf::Vec3d> calculatePointNormals(Rim3dWellLogCurve::DrawPlane   drawPlane,
                                                         const std::vector<cvf::Vec3d>& wellPathPoints);

private:
    cvf::ref<RigWellPath> m_wellPathGeometry;
};
