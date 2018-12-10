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
#include "cvfObject.h"
#include "cvfVector3.h"

#include "Rim3dWellLogCurve.h"
#include "Rim3dWellLogCurveCollection.h"

#include "cafPdmPointer.h"


namespace cvf
{
class ModelBasicList;
class Drawable;
class Effect;
class Part;
class BoundingBox;
class Color3f;
}

namespace caf
{
class DisplayCoordTransform;
}

class RimGridView;
class RimWellPath;
class Riv3dWellLogDrawSurfaceGenerator;

class Riv3dWellLogPlanePartMgr : public cvf::Object
{
public:
    Riv3dWellLogPlanePartMgr(RimWellPath* wellPath, RimGridView* gridView);

    void appendPlaneToModel(cvf::ModelBasicList*              model,
                            const caf::DisplayCoordTransform* displayCoordTransform,
                            const cvf::BoundingBox&           wellPathClipBoundingBox,
                            bool                              isStaticResult = false);
private:
    void append3dWellLogCurveToModel(cvf::ModelBasicList*              model,
                                     const caf::DisplayCoordTransform* displayCoordTransform,
                                     const cvf::BoundingBox&           wellPathClipBoundingBox,
                                     Rim3dWellLogCurve*                rim3dWellLogCurve,
                                     const std::vector<cvf::Vec3d>&    drawSurfaceVertices);

    void appendDrawSurfaceToModel(cvf::ModelBasicList*              model,
                                  const caf::DisplayCoordTransform* displayCoordTransform,
                                  const cvf::BoundingBox&           wellPathClipBoundingBox,
                                  Rim3dWellLogCurve::DrawPlane      drawPlane,
                                  double                            samplingInterval);

    cvf::ref<cvf::Part> createPart(cvf::Drawable* drawable, cvf::Effect* effect);

    double wellPathCenterToPlotStartOffset(Rim3dWellLogCurve::DrawPlane drawPlane) const;
    double planeWidth() const;

private:
    std::map<Rim3dWellLogCurve::DrawPlane, cvf::ref<Riv3dWellLogDrawSurfaceGenerator>> m_3dWellLogDrawSurfaceGeometryGenerators;
    
    caf::PdmPointer<RimWellPath>                m_wellPath;
    caf::PdmPointer<RimGridView>                m_gridView;
};
