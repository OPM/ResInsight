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
namespace RigWellPathGeometryTools
{
enum VertexOrganization
{
    LINE_SEGMENTS,
    POLYLINE
};

std::vector<cvf::Vec3d> calculateLineSegmentNormals( const std::vector<cvf::Vec3d>& vertices, double angle );
std::vector<double>     interpolateMdFromTvd( const std::vector<double>& originalMdValues,
                                              const std::vector<double>& originalTvdValues,
                                              const std::vector<double>& tvdValuesToInterpolateFrom );

std::pair<double, double> calculateAzimuthAndInclinationAtMd( double measuredDepth, gsl::not_null<const RigWellPath*> wellPathGeometry );

std::vector<double> calculateMeasuredDepth( const std::vector<cvf::Vec3d>& wellPathPoints, double startMd = 0.0 );
} // namespace RigWellPathGeometryTools
