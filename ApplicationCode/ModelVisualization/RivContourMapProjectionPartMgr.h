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

#include "cafPdmPointer.h"
#include "cafDisplayCoordTransform.h"

#include "cvfBase.h"
#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfObject.h"

class RimContourMapView;
class RimContourMapProjection;

class RivContourMapProjectionPartMgr : public cvf::Object
{
public:
    RivContourMapProjectionPartMgr(RimContourMapProjection* contourMapProjection, RimContourMapView* contourMap);

    void appendProjectionToModel(cvf::ModelBasicList*              model,
                                 const caf::DisplayCoordTransform* displayCoordTransform) const;
    void appendPickPointVisToModel(cvf::ModelBasicList* model,
                                   const caf::DisplayCoordTransform* displayCoordTransform) const;

    cvf::ref<cvf::Vec2fArray> createTextureCoords() const;

    void removeTrianglesWithNoResult(cvf::UIntArray* uintArray) const;
private:
    cvf::ref<cvf::DrawableGeo>              createProjectionMapDrawable(const caf::DisplayCoordTransform* displayCoordTransform) const;
    std::vector<cvf::ref<cvf::DrawableGeo>> createContourPolygons(const caf::DisplayCoordTransform* displayCoordTransform) const;
    cvf::ref<cvf::DrawableGeo>              createPickPointVisDrawable(const caf::DisplayCoordTransform* displayCoordTransform) const;
private:
    caf::PdmPointer<RimContourMapProjection>    m_contourMapProjection;
    caf::PdmPointer<RimContourMapView> m_parentContourMap;
};

