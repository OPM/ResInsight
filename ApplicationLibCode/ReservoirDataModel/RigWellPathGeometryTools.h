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

#include <qwt_curve_fitter.h>
#include <qwt_spline.h>

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

private:
    static std::vector<cvf::Vec3d> interpolateUndefinedNormals( const cvf::Vec3d&              planeNormal,
                                                                const std::vector<cvf::Vec3d>& normals,
                                                                const std::vector<cvf::Vec3d>& vertices );
    static cvf::Vec3d              estimateDominantDirectionInXYPlane( const std::vector<cvf::Vec3d>& vertices );

    static double solveForX( const QwtSpline& spline, double minX, double maxX, double y );

    static QwtSpline        createSpline( const std::vector<double>& originalMdValues,
                                          const std::vector<double>& originalTvdValues );
    static std::vector<int> findSplineSegmentsContainingRoots( const QwtSpline&           spline,
                                                               const std::vector<double>& tvdValuesToInterpolateFrom );
};
