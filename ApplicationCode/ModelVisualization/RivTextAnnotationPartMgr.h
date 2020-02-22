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

#include "cafPdmPointer.h"
#include "cvfAssert.h"
#include "cvfObject.h"

#include <cvfVector3.h>

namespace cvf
{
class BoundingBox;
class Part;
class ModelBasicList;
class Transform;
class Font;
} // namespace cvf
namespace caf
{
class DisplayCoordTransform;
}

class Rim3dView;
class RimAnnotationInViewCollection;
class RimTextAnnotation;
class RimTextAnnotationInView;

class RivTextAnnotationPartMgr : public cvf::Object
{
    using Vec3d = cvf::Vec3d;

public:
    RivTextAnnotationPartMgr( Rim3dView* view, RimTextAnnotation* annotationLocal );
    RivTextAnnotationPartMgr( Rim3dView* view, RimTextAnnotationInView* annotationInView );
    ~RivTextAnnotationPartMgr() override;

    void appendDynamicGeometryPartsToModel( cvf::ModelBasicList*              model,
                                            const caf::DisplayCoordTransform* displayXf,
                                            const cvf::BoundingBox&           boundingBox );

private:
    void buildParts( const caf::DisplayCoordTransform* displayXf, bool doFlatten, double xOffset );

    Vec3d getAnchorPointInDomain( bool snapToPlaneZ, double planeZ );
    Vec3d getLabelPointInDomain( bool snapToPlaneZ, double planeZ );

    bool isTextInBoundingBox( const cvf::BoundingBox& boundingBox );

    void clearAllGeometry();
    bool validateAnnotation( const RimTextAnnotation* annotation ) const;

    RimAnnotationInViewCollection* annotationCollection() const;

    RimTextAnnotation* rimAnnotation() const;
    bool               isAnnotationVisible() const;

    caf::PdmPointer<Rim3dView>               m_rimView;
    caf::PdmPointer<RimTextAnnotation>       m_rimAnnotationLocal;
    caf::PdmPointer<RimTextAnnotationInView> m_rimAnnotationInView;
    cvf::ref<cvf::Part>                      m_linePart;
    cvf::ref<cvf::Part>                      m_labelPart;
};
