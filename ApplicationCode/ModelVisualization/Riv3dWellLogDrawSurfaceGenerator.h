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
#include "cvfDrawableVectors.h"
#include "cvfObject.h"

#include "cafPdmPointer.h"

#include <map>

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

class Riv3dWellLogDrawSurfaceGenerator : public cvf::Object
{
public:
    Riv3dWellLogDrawSurfaceGenerator(RimWellPath* wellPath);

    bool createDrawSurface(const caf::DisplayCoordTransform* displayCoordTransform,
                           const cvf::BoundingBox&           wellPathClipBoundingBox,
                           double                            planeAngle,
                           double                            planeOffsetFromWellPathCenter,
                           double                            planeWidth,
                           double                            samplingIntervalSize);

    void clearGeometry();

    cvf::ref<cvf::DrawableGeo> background() const;
    cvf::ref<cvf::DrawableGeo> border() const;
    cvf::ref<cvf::DrawableVectors> curveNormalVectors() const;

    const std::vector<cvf::Vec3d>& vertices() const;

private:
    void               createCurveNormalVectors(const caf::DisplayCoordTransform* displayCoordTransform, 
                                                size_t                         clipStartIndex,
                                                double                         planeOffsetFromWellPathCenter,
                                                double                         planeWidth,
                                                double                         samplingIntervalSize,
                                                const std::vector<cvf::Vec3d>& wellPathSegmentNormals);
    
    void               createBackground(cvf::Vec3fArray* vertexArray);
    void               createBorder(cvf::Vec3fArray* vertexArray);

    const RigWellPath* wellPathGeometry() const;

private:
    caf::PdmPointer<RimWellPath>     m_wellPath;
    cvf::ref<cvf::DrawableGeo>       m_background;
    cvf::ref<cvf::DrawableGeo>       m_border;
    cvf::ref<cvf::DrawableVectors>   m_curveNormalVectors;

    std::vector<cvf::Vec3d>          m_vertices;
};
