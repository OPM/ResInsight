/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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
#include "cvfAssert.h"
#include "cvfObject.h"
#include "cafPdmPointer.h"

#include "cvfVector3.h"

#include <vector>

namespace cvf
{
    class BoundingBox;
    class Part;
    class ModelBasicList;
    class Transform;
    class Font;
}
namespace caf
{
    class DisplayCoordTransform;
}

class Rim3dView;
class RimReachCircleAnnotation;
class RimAnnotationInViewCollection;

class RivReachCircleAnnotationPartMgr : public cvf::Object
{
    using Vec3d = cvf::Vec3d;

public:
    RivReachCircleAnnotationPartMgr(Rim3dView* view, RimReachCircleAnnotation* annotation);
    ~RivReachCircleAnnotationPartMgr() override;

    void appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model,
                                           const caf::DisplayCoordTransform* displayXf,
                                           const cvf::BoundingBox& boundingBox);

private:
    void buildParts(const caf::DisplayCoordTransform* displayXf, bool doFlatten, double xOffset);

    void clearAllGeometry();
    bool validateAnnotation(const RimReachCircleAnnotation* annotation) const;
    bool isCircleInBoundingBox(const cvf::BoundingBox& boundingBox);

    std::vector<Vec3d> computeCirclePointsInDomain(bool snapToPlaneZ, double planeZ);
    std::vector<Vec3d> transformCirclePointsToDisplay(const std::vector<Vec3d>& pointsInDomain,
                                                      const caf::DisplayCoordTransform* displayXf);

    RimAnnotationInViewCollection* annotationCollection() const;

    caf::PdmPointer<Rim3dView>                  m_rimView;
    caf::PdmPointer<RimReachCircleAnnotation>   m_rimAnnotation;
    cvf::ref<cvf::Part>                         m_circlePart;
    cvf::ref<cvf::Part>                         m_centerPointPart;
};
