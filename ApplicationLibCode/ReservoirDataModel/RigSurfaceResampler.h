/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RigSurface.h"

#include "cvfObject.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RigSurfaceResampler
{
public:
    static cvf::ref<RigSurface> resampleSurface( cvf::ref<RigSurface> targetSurface, cvf::ref<RigSurface> surface );

    static bool resamplePoint( RigSurface*       surface,
                               const cvf::Vec3d& pointAbove,
                               const cvf::Vec3d& pointBelow,
                               cvf::Vec3d&       intersectionPoint );

private:
    static bool findClosestPointXY( const cvf::Vec3d&              targetPoint,
                                    const std::vector<cvf::Vec3d>& vertices,
                                    double                         maxDistance,
                                    cvf::Vec3d&                    intersectionPoint );
};
