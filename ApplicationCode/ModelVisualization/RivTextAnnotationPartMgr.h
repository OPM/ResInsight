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
class RimAnnotationInViewCollection;
class RimTextAnnotation;
class RimSimWellInView;
class RimSimWellInViewCollection;

class RivTextAnnotationPartMgr : public cvf::Object
{
public:
    RivTextAnnotationPartMgr(Rim3dView* view, RimTextAnnotation* annotation);
    ~RivTextAnnotationPartMgr() override;

    void appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, 
                                           const caf::DisplayCoordTransform * displayXf);

private:
    void                            buildParts(const caf::DisplayCoordTransform * displayXf, 
                                               bool doFlatten, 
                                               double xOffset);

    void                            clearAllGeometry();
    bool                            validateAnnotation(const RimTextAnnotation* annotation) const;

    RimAnnotationInViewCollection*  annotationCollection() const;

    caf::PdmPointer<Rim3dView>          m_rimView;
    caf::PdmPointer<RimTextAnnotation>  m_rimAnnotation;
    cvf::ref<cvf::Part>                 m_linePart;
    cvf::ref< cvf::Part >               m_labelPart;
};
