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

#include "cvfVector3.h"

#include <QPolygon>

#include <gsl/gsl>
#include <vector>

class RigWellPath;

//==================================================================================================
///
//==================================================================================================
class RigWellPathGeometryTools
{
public:
    enum VertexOrganization
    {
        LINE_SEGMENTS,
        POLYLINE
    };

public:
    static std::vector<cvf::Vec3d> calculateLineSegmentNormals( const std::vector<cvf::Vec3d>& vertices, double angle );
    static std::vector<double>     interpolateMdFromTvd( const std::vector<double>& originalMdValues,
                                                         const std::vector<double>& originalTvdValues,
                                                         const std::vector<double>& tvdValuesToInterpolateFrom );

    static std::pair<double, double>
        calculateAzimuthAndInclinationAtMd( double measuredDepth, gsl::not_null<const RigWellPath*> wellPathGeometry );

private:
    static std::vector<cvf::Vec3d> interpolateUndefinedNormals( const cvf::Vec3d&              planeNormal,
                                                                const std::vector<cvf::Vec3d>& normals,
                                                                const std::vector<cvf::Vec3d>& vertices );
    static cvf::Vec3d              estimateDominantDirectionInXYPlane( const std::vector<cvf::Vec3d>& vertices );

    static double solveForX( const QPolygonF& spline, double minX, double maxX, double y );

    static QPolygonF createSplinePoints( const std::vector<double>& originalMdValues,
                                         const std::vector<double>& originalTvdValues );

    static std::vector<int> findSplineSegmentsContainingRoots( const QPolygonF&           points,
                                                               const std::vector<double>& tvdValuesToInterpolateFrom );

    // Temporary helper function to method removed from Qwt >= 6.2
    static int    lookup( double x, const QPolygonF& values );
    static double value( double x, const QPolygonF& values );
};
