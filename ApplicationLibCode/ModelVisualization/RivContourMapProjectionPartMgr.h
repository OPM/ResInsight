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

#include "ContourMap/RigContourPolygonsTools.h"

#include "RiaNumberFormat.h"

#include "cafDisplayCoordTransform.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"

#include "cvfDrawableGeo.h"
#include "cvfDrawableText.h"
#include "cvfObject.h"
#include "cvfVector2.h"
#include "cvfVector4.h"

class RigContourMapGrid;

namespace cvf
{
class Effect;
class ScalarMapper;
class Color3f;
class ModelBasicList;
class Part;
} // namespace cvf

class RivContourMapProjectionPartMgr : public cvf::Object
{
public:
    RivContourMapProjectionPartMgr( caf::PdmObject* contourMapProjection );

    void appendProjectionToModel( cvf::ModelBasicList*              model,
                                  const caf::DisplayCoordTransform* displayCoordTransform,
                                  const std::vector<cvf::Vec4d>&    vertices,
                                  const RigContourMapGrid&          contourMapGrid,
                                  const cvf::Color3f&               backgroundColor,
                                  cvf::ScalarMapper*                scalarMapper ) const;

    void appendContourLinesToModel( const cvf::Camera*                                           camera,
                                    cvf::ModelBasicList*                                         model,
                                    const caf::DisplayCoordTransform*                            displayCoordTransform,
                                    const std::vector<RigContourPolygonsTools::ContourPolygons>& contourLinePolygons,
                                    const RigContourMapGrid&                                     contourMapGrid,
                                    cvf::ScalarMapper*                                           mapper,
                                    bool                                                         showContourLines,
                                    bool                                                         showContourLabels,
                                    RiaNumberFormat::NumberFormatType                            numberFormat,
                                    int                                                          precision );

    void appendPickPointVisToModel( cvf::ModelBasicList*              model,
                                    const caf::DisplayCoordTransform* displayCoordTransform,
                                    const cvf::Vec2d&                 pickPoint,
                                    const RigContourMapGrid&          contourMapGrid ) const;

    cvf::ref<cvf::Vec2fArray> createTextureCoords( const std::vector<double>& values, cvf::ScalarMapper* scalarMapper ) const;

private:
    static cvf::ref<cvf::DrawableText> createTextLabel( const cvf::Color3f& textColor, const cvf::Color3f& backgroundColor );
    cvf::ref<cvf::Part>                createProjectionMapPart( const caf::DisplayCoordTransform* displayCoordTransform,
                                                                const std::vector<cvf::Vec4d>&    vertices,
                                                                const RigContourMapGrid&          contourMapGrid,
                                                                const cvf::Color3f&               backgroundColor,
                                                                cvf::ScalarMapper*                scalarMapper ) const;

    std::vector<std::vector<cvf::ref<cvf::Drawable>>>
        createContourPolygons( const caf::DisplayCoordTransform*                            displayCoordTransform,
                               const std::vector<std::vector<cvf::BoundingBox>>&            labelBBoxes,
                               const std::vector<RigContourPolygonsTools::ContourPolygons>& contourLinePolygons,
                               cvf::ScalarMapper*                                           scalarMapper,
                               const RigContourMapGrid&                                     contourMapGrid ) const;

    std::vector<cvf::ref<cvf::Drawable>> createContourLabels( const cvf::Camera*                          camera,
                                                              const caf::DisplayCoordTransform*           displayCoordTransform,
                                                              std::vector<std::vector<cvf::BoundingBox>>* labelBBoxes,
                                                              const std::vector<RigContourPolygonsTools::ContourPolygons>& contourLinePolygons,
                                                              const RigContourMapGrid&          contourMapGrid,
                                                              const cvf::ScalarMapper*          scalarMapper,
                                                              RiaNumberFormat::NumberFormatType numberFormat,
                                                              int                               precision ) const;

    cvf::ref<cvf::DrawableGeo> createPickPointVisDrawable( const caf::DisplayCoordTransform* displayCoordTransform,
                                                           const cvf::Vec2d&                 pickPoint,
                                                           const RigContourMapGrid&          contourMapGrid ) const;

    static bool lineOverlapsWithPreviousContourLevel( const cvf::Vec3d&                               lineCenter,
                                                      const RigContourPolygonsTools::ContourPolygons& previousLevel,
                                                      double                                          tolerance );

private:
    caf::PdmPointer<caf::PdmObject> m_pdmObject;

    std::vector<std::vector<cvf::BoundingBox>> m_labelBoundingBoxes;
    cvf::ref<cvf::Effect>                      m_labelEffect;
};
