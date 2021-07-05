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
#include "RigSurfaceResampler.h"

#include "cvfGeometryTools.h"

#include "cvfObject.h"
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigSurface> RigSurfaceResampler::resampleSurface( cvf::ref<RigSurface> targetSurface, cvf::ref<RigSurface> surface )
{
    cvf::ref<RigSurface> resampledSurface = cvf::make_ref<RigSurface>();

    const std::vector<cvf::Vec3d>& targetVerts   = targetSurface->vertices();
    const std::vector<unsigned>&   targetIndices = targetSurface->triangleIndices();

    std::vector<cvf::Vec3d> resampledVerts;

    for ( auto targetVert : targetVerts )
    {
        cvf::Vec3d pointAbove = cvf::Vec3d( targetVert.x(), targetVert.y(), 10000.0 );
        cvf::Vec3d pointBelow = cvf::Vec3d( targetVert.x(), targetVert.y(), -10000.0 );

        cvf::Vec3d intersectionPoint;
        bool       foundMatch =
            resamplePoint( pointAbove, pointBelow, surface->triangleIndices(), surface->vertices(), intersectionPoint );
        if ( !foundMatch )
            intersectionPoint = cvf::Vec3d( targetVert.x(), targetVert.y(), std::numeric_limits<double>::infinity() );

        resampledVerts.push_back( intersectionPoint );
    }

    resampledSurface->setTriangleData( targetIndices, resampledVerts );

    return resampledSurface;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigSurfaceResampler::resamplePoint( const cvf::Vec3d&                pointAbove,
                                         const cvf::Vec3d&                pointBelow,
                                         const std::vector<unsigned int>& indices,
                                         const std::vector<cvf::Vec3d>&   vertices,
                                         cvf::Vec3d&                      intersectionPoint )
{
    for ( size_t i = 0; i < indices.size(); i += 3 )
    {
        bool isLineDirDotNormalNegative = false;
        if ( cvf::GeometryTools::intersectLineSegmentTriangle( pointAbove,
                                                               pointBelow,
                                                               vertices[indices[i]],
                                                               vertices[indices[i + 1]],
                                                               vertices[indices[i + 2]],
                                                               &intersectionPoint,
                                                               &isLineDirDotNormalNegative ) == 1 )
            return true;
    }

    return false;
}
