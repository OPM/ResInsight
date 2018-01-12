/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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
#include "cvfMatrix4.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include "cafPdmPointer.h"

#include <vector>

namespace cvf
{
     class ModelBasicList;
     class DrawableGeo;
     class Part;
     class Color3f;
}

namespace caf
{
    class DisplayCoordTransform;
}

class RimFracture;
class RimStimPlanFractureTemplate;
class RimEclipseView;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RivWellFracturePartMgr : public cvf::Object
{
public:
    RivWellFracturePartMgr(RimFracture* well);
    ~RivWellFracturePartMgr();

    void                                appendGeometryPartsToModel(cvf::ModelBasicList* model, const RimEclipseView& eclView);

    static std::vector<double>          mirrorDataAtSingleDepth(std::vector<double> depthData);

private:
    cvf::ref<cvf::Part>                 createEllipseSurfacePart(const RimEclipseView& activeView);
    cvf::ref<cvf::Part>                 createStimPlanColorInterpolatedSurfacePart(const RimEclipseView& activeView);

    cvf::ref<cvf::Part>                 createSingleColorSurfacePart(const std::vector<cvf::uint>& triangleIndices, const std::vector<cvf::Vec3f>& nodeCoords, const cvf::Color3f& color);

    cvf::ref<cvf::Part>                 createStimPlanElementColorSurfacePart(const RimEclipseView& activeView);

    cvf::ref<cvf::Part>                 createContainmentMaskPart(const RimEclipseView& activeView);

    cvf::ref<cvf::Part>                 createStimPlanMeshPart(const RimEclipseView& activeView);
    cvf::ref<cvf::DrawableGeo>          createStimPlanMeshDrawable(RimStimPlanFractureTemplate* stimPlanFracTemplate, const RimEclipseView& activeView) const;

    static std::vector<cvf::Vec3f>      transformToFractureDisplayCoords(const std::vector<cvf::Vec3f>& polygon, 
                                                                         cvf::Mat4d m, 
                                                                         const caf::DisplayCoordTransform& displayCoordTransform);

    static bool                         stimPlanCellTouchesPolygon(const std::vector<cvf::Vec3f>& polygon, 
                                                                   double xMin, 
                                                                   double xMax, 
                                                                   double yMin, 
                                                                   double yMax, 
                                                                   float polygonXmin, 
                                                                   float polygonXmax, 
                                                                   float polygonYmin, 
                                                                   float polygonYmax);

    static cvf::ref<cvf::DrawableGeo>   buildDrawableGeoFromTriangles(const std::vector<cvf::uint>& triangleIndices, const std::vector<cvf::Vec3f>& nodeCoords);

private:
    caf::PdmPointer<RimFracture>        m_rimFracture;
};
