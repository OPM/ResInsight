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

    // Handle cases where no match is found due to floating point imprecision,
    // or when falling off resulting grid slightly.
    double maxDistance = 10.0;
    return findClosestPointXY( pointAbove, vertices, maxDistance, intersectionPoint );
}

//--------------------------------------------------------------------------------------------------
/// Find the closest vertex to targetPoint (must be closer than maxDistance) in XY plane.
/// Unit maxDistance: meter.
//--------------------------------------------------------------------------------------------------
bool RigSurfaceResampler::findClosestPointXY( const cvf::Vec3d&              targetPoint,
                                              const std::vector<cvf::Vec3d>& vertices,
                                              double                         maxDistance,
                                              cvf::Vec3d&                    intersectionPoint )
{
    // Find closest vertices
    double shortestDistance = std::numeric_limits<double>::max();
    double closestZ         = std::numeric_limits<double>::infinity();
    for ( auto v : vertices )
    {
        // Ignore height (z) component when finding closest by
        // moving point to same height as target point above
        cvf::Vec3d p( v.x(), v.y(), targetPoint.z() );
        double     distance = p.pointDistance( targetPoint );
        if ( distance < shortestDistance )
        {
            shortestDistance = distance;
            closestZ         = v.z();
        }
    }

    // Check if the closest point is not to far away to be valid
    if ( shortestDistance < maxDistance )
    {
        intersectionPoint = cvf::Vec3d( targetPoint.x(), targetPoint.y(), closestZ );
        return true;
    }
    else
    {
        return false;
    }
}
