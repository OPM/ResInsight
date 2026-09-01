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

#include "cafDisplayCoordTransform.h"
#include "cafFontTools.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafPdmUiNumberFormat.h"

#include "cvfColor3.h"
#include "cvfDrawableGeo.h"
#include "cvfDrawableText.h"
#include "cvfObject.h"
#include "cvfVector2.h"
#include "cvfVector4.h"

#include <optional>

class RigContourMapGrid;
class RivContourMapElevationProvider;

namespace cvf
{
class Effect;
class ScalarMapper;
class Color3f;
class ModelBasicList;
class Part;
} // namespace cvf

//==================================================================================================
///
/// How the contour lines are drawn.
///
/// The defaults reproduce the appearance used by the 2d contour map views: one hairline per contour
/// level, coloured to contrast the level colour of the map underneath it.
///
//==================================================================================================
struct RivContourLineAppearance
{
    // When not set, every contour level gets a colour contrasting its own level colour
    std::optional<cvf::Color3f> color;
    float                       lineWidth = 1.0f;

    caf::FontTools::FontSize labelFontSize = caf::FontTools::FontSize::FONT_SIZE_10;

    // Labels are drawn along the contour segment they belong to. The direction is a screen space one,
    // which only lines up with the segment while the camera looks straight down at the map, so in a 3d
    // view the text ends up rotated arbitrarily. Set this to keep the labels upright on screen instead.
    bool alignLabelsWithCamera = false;

    // The colour to draw a contour level in. Shared by the lines and their labels so the two cannot
    // drift apart. levelColor is the map colour of the level, used when no explicit colour is set.
    cvf::Color3f colorForLevel( const cvf::Color3f& levelColor ) const;
};

class RivContourMapProjectionPartMgr : public cvf::Object
{
public:
    RivContourMapProjectionPartMgr( caf::PdmObject* contourMapProjection );

    // elevationProvider controls where the geometry is placed vertically. When null, the geometry is
    // placed on the horizontal plane at RigContourMapGrid::origin3d(), which is the behaviour used by
    // the 2d contour map views.
    void appendProjectionToModel( cvf::ModelBasicList*                  model,
                                  const caf::DisplayCoordTransform*     displayCoordTransform,
                                  const std::vector<cvf::Vec4d>&        vertices,
                                  const RigContourMapGrid&              contourMapGrid,
                                  const cvf::Color3f&                   backgroundColor,
                                  cvf::ScalarMapper*                    scalarMapper,
                                  const RivContourMapElevationProvider* elevationProvider = nullptr ) const;

    void appendContourLinesToModel( const cvf::Camera*                                           camera,
                                    cvf::ModelBasicList*                                         model,
                                    const caf::DisplayCoordTransform*                            displayCoordTransform,
                                    const std::vector<RigContourPolygonsTools::ContourPolygons>& contourLinePolygons,
                                    const RigContourMapGrid&                                     contourMapGrid,
                                    cvf::ScalarMapper*                                           mapper,
                                    bool                                                         showContourLines,
                                    bool                                                         showContourLabels,
                                    caf::NumberFormatType                                        numberFormat,
                                    int                                                          precision,
                                    const RivContourMapElevationProvider*                        elevationProvider = nullptr,
                                    const RivContourLineAppearance&                              lineAppearance    = {} );

    void appendPickPointVisToModel( cvf::ModelBasicList*              model,
                                    const caf::DisplayCoordTransform* displayCoordTransform,
                                    const cvf::Vec2d&                 pickPoint,
                                    const RigContourMapGrid&          contourMapGrid ) const;

    cvf::ref<cvf::Vec2fArray> createTextureCoords( const std::vector<double>& values, cvf::ScalarMapper* scalarMapper ) const;

private:
    // Convert a contour map local vertex to domain coordinates. Without an elevation provider this is
    // the vertex offset by RigContourMapGrid::origin3d(). With one, the z component is replaced by the
    // elevation reported for the vertex position, and nothing is returned where the provider has no
    // elevation to offer.
    static std::optional<cvf::Vec3d> toDomainCoord( const cvf::Vec3d&                     localVertex,
                                                    const RigContourMapGrid&              contourMapGrid,
                                                    const RivContourMapElevationProvider* elevationProvider );

    static std::optional<cvf::Vec3d> toDisplayCoord( const cvf::Vec3d&                     localVertex,
                                                     const RigContourMapGrid&              contourMapGrid,
                                                     const caf::DisplayCoordTransform*     displayCoordTransform,
                                                     const RivContourMapElevationProvider* elevationProvider );

    // The segments of a closed contour polygon in display coordinates, sampled densely enough to follow
    // the elevation reported by the provider. Segments where no elevation is defined are left out.
    static std::vector<std::pair<cvf::Vec3d, cvf::Vec3d>> resampledDisplaySegments( const std::vector<cvf::Vec3d>&    localVertices,
                                                                                    const RigContourMapGrid&          contourMapGrid,
                                                                                    const caf::DisplayCoordTransform* displayCoordTransform,
                                                                                    const RivContourMapElevationProvider* elevationProvider );

    static cvf::ref<cvf::DrawableText>
        createTextLabel( const cvf::Color3f& textColor, const cvf::Color3f& backgroundColor, caf::FontTools::FontSize fontSize );
    cvf::ref<cvf::Part> createProjectionMapPart( const caf::DisplayCoordTransform*     displayCoordTransform,
                                                 const std::vector<cvf::Vec4d>&        vertices,
                                                 const RigContourMapGrid&              contourMapGrid,
                                                 const cvf::Color3f&                   backgroundColor,
                                                 cvf::ScalarMapper*                    scalarMapper,
                                                 const RivContourMapElevationProvider* elevationProvider ) const;

    std::vector<std::vector<cvf::ref<cvf::Drawable>>>
        createContourPolygons( const caf::DisplayCoordTransform*                            displayCoordTransform,
                               const std::vector<std::vector<cvf::BoundingBox>>&            labelBBoxes,
                               const std::vector<RigContourPolygonsTools::ContourPolygons>& contourLinePolygons,
                               cvf::ScalarMapper*                                           scalarMapper,
                               const RigContourMapGrid&                                     contourMapGrid,
                               const RivContourMapElevationProvider*                        elevationProvider ) const;

    std::vector<cvf::ref<cvf::Drawable>> createContourLabels( const cvf::Camera*                          camera,
                                                              const caf::DisplayCoordTransform*           displayCoordTransform,
                                                              std::vector<std::vector<cvf::BoundingBox>>* labelBBoxes,
                                                              const std::vector<RigContourPolygonsTools::ContourPolygons>& contourLinePolygons,
                                                              const RigContourMapGrid&              contourMapGrid,
                                                              const cvf::ScalarMapper*              scalarMapper,
                                                              caf::NumberFormatType                 numberFormat,
                                                              int                                   precision,
                                                              const RivContourMapElevationProvider* elevationProvider,
                                                              const RivContourLineAppearance&       lineAppearance ) const;

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
