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

namespace cvf
{
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
class RimSimWellInView;
class RimSimWellInViewCollection;

class RivReachCircleAnnotationPartMgr : public cvf::Object
{
public:
    RivReachCircleAnnotationPartMgr( RimReachCircleAnnotation* annotation);
    ~RivReachCircleAnnotationPartMgr() override;

    void appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, 
                                           const caf::DisplayCoordTransform * displayXf);

    void appendFlattenedDynamicGeometryPartsToModel(cvf::ModelBasicList* model, 
                                                    size_t frameIndex,
                                                    const caf::DisplayCoordTransform * displayXf, 
                                                    double xOffset);



private:

    void                            clearAllGeometry();
    Rim3dView*                      viewWithSettings();
    RimAnnotationInViewCollection*  annotatationInViewCollection();
    bool                            validateAnnotation(const RimReachCircleAnnotation* annotation) const;

    caf::PdmPointer<RimReachCircleAnnotation>   m_rimAnnotation;
    cvf::ref< cvf::Part >                       m_part;
};
