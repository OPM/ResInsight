/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "cvfCollection.h"
#include "cvfColor3.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include <QString>

#include <functional>
#include <map>
#include <vector>

class RimSurface;
class RimSurfaceIntersectionCollection;

namespace cvf
{
class Part;
class Transform;
} // namespace cvf

//==================================================================================================
/// The curve created when a surface is projected onto a vertical intersection. One entry per
/// resampled footprint point, where valid[i] is false if the vertical ray missed the surface or the
/// hit fell outside the depth extent of the intersection. Invalid points are kept in the array
/// instead of being removed, so the two curves of a band stay index aligned.
//==================================================================================================
class RivSurfaceCurtainPolyline
{
public:
    std::vector<cvf::Vec3d> points;
    std::vector<bool>       valid;

    /// The stretches of consecutive valid points that are long enough to draw a line
    std::vector<std::vector<cvf::Vec3d>> validRuns() const;
};

//==================================================================================================
/// Creation of surface intersection curves and bands. Shared by all intersection types that produce
/// a vertical curtain, and that therefore can find the curve by shooting a vertical ray at the
/// surface for each point along the footprint of the intersection.
//==================================================================================================
class RivSurfaceIntersectionCurveTools
{
public:
    static std::vector<RimSurface*> referencedSurfaces( const RimSurfaceIntersectionCollection* surfaceIntersections );

    /// pointTransform maps (worldPoint, footprintSegmentIndex) to the output coordinate system. Use
    /// the identity for intersections drawn in 3D only, or the flattening transform when the curve
    /// is also displayed in a 2D intersection view.
    static std::map<RimSurface*, RivSurfaceCurtainPolyline>
        computeSurfaceCurtainPolylines( const std::vector<RimSurface*>&                               surfaces,
                                        const std::vector<cvf::Vec3d>&                                footprintPolyline,
                                        double                                                        minZ,
                                        double                                                        maxZ,
                                        const std::function<cvf::Vec3d( const cvf::Vec3d&, size_t )>& pointTransform );

    static cvf::Collection<cvf::Part> createAnnotationParts( const RimSurfaceIntersectionCollection*                 surfaceIntersections,
                                                             const std::map<RimSurface*, RivSurfaceCurtainPolyline>& surfacePolylines,
                                                             cvf::Transform*                                         scaleTransform );

private:
    static cvf::Collection<cvf::Part> createCurveParts( const RivSurfaceCurtainPolyline& polyline,
                                                        const QString&                   description,
                                                        const cvf::Color3f&              color,
                                                        float                            lineWidth,
                                                        cvf::Transform*                  scaleTransform );

    static cvf::ref<cvf::Part> createCurvePart( const std::vector<cvf::Vec3d>& polyline, const cvf::Color3f& color, float lineWidth );

    static cvf::ref<cvf::Part> createBandPart( const RivSurfaceCurtainPolyline& polylineA,
                                               const RivSurfaceCurtainPolyline& polylineB,
                                               const cvf::Color3f&              color,
                                               float                            opacity,
                                               double                           polygonOffsetUnit );
};
