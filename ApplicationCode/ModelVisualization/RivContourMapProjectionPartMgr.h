/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RimEclipseContourMapProjection.h"

#include "cafDisplayCoordTransform.h"
#include "cafPdmPointer.h"

#include "cvfDrawableGeo.h"
#include "cvfDrawableText.h"
#include "cvfModelBasicList.h"
#include "cvfObject.h"
#include "cvfVector4.h"

class RimEclipseContourMapView;

namespace cvf
{
class Effect;
}

class RivContourMapProjectionPartMgr : public cvf::Object
{
public:
    RivContourMapProjectionPartMgr( RimContourMapProjection* contourMapProjection, RimGridView* contourMap );

    void createProjectionGeometry();
    void appendProjectionToModel( cvf::ModelBasicList* model, const caf::DisplayCoordTransform* displayCoordTransform ) const;
    void appendContourLinesToModel( const cvf::Camera*                camera,
                                    cvf::ModelBasicList*              model,
                                    const caf::DisplayCoordTransform* displayCoordTransform );
    void appendPickPointVisToModel( cvf::ModelBasicList*              model,
                                    const caf::DisplayCoordTransform* displayCoordTransform ) const;

    cvf::ref<cvf::Vec2fArray> createTextureCoords( const std::vector<double>& values ) const;

private:
    static cvf::ref<cvf::DrawableText> createTextLabel( const cvf::Color3f& textColor, const cvf::Color3f& backgroundColor );
    cvf::ref<cvf::Part> createProjectionMapPart( const caf::DisplayCoordTransform* displayCoordTransform ) const;
    std::vector<std::vector<cvf::ref<cvf::Drawable>>>
                                         createContourPolygons( const caf::DisplayCoordTransform*                 displayCoordTransform,
                                                                const std::vector<std::vector<cvf::BoundingBox>>& labelBBoxes ) const;
    std::vector<cvf::ref<cvf::Drawable>> createContourLabels( const cvf::Camera*                camera,
                                                              const caf::DisplayCoordTransform* displayCoordTransform,
                                                              std::vector<std::vector<cvf::BoundingBox>>* labelBBoxes ) const;
    cvf::ref<cvf::DrawableGeo> createPickPointVisDrawable( const caf::DisplayCoordTransform* displayCoordTransform ) const;
    bool                       lineOverlapsWithPreviousContourLevel( const cvf::Vec3d&                               lineCenter,
                                                                     const RimContourMapProjection::ContourPolygons* previousLevel ) const;

private:
    caf::PdmPointer<RimContourMapProjection> m_contourMapProjection;
    caf::PdmPointer<RimGridView>             m_parentContourMap;

    std::vector<RimContourMapProjection::ContourPolygons> m_contourLinePolygons;
    std::vector<cvf::Vec4d>                               m_contourMapTriangles;
    std::vector<std::vector<cvf::BoundingBox>>            m_labelBoundingBoxes;
    cvf::ref<cvf::Effect>                                 m_labelEffect;
};
