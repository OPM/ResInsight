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

#include "cvfBoundingBox.h"
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
        bool       foundMatch = resamplePoint( surface.p(), pointAbove, pointBelow, intersectionPoint );
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
bool RigSurfaceResampler::resamplePoint( RigSurface*       surface,
                                         const cvf::Vec3d& pointAbove,
                                         const cvf::Vec3d& pointBelow,
                                         cvf::Vec3d&       intersectionPoint )
{
    surface->ensureIntersectionSearchTreeIsBuilt();

    cvf::BoundingBox bb;
    bb.add( pointAbove );
    bb.add( pointBelow );

    std::vector<size_t> triangleStartIndices;
    surface->findIntersectingTriangles( bb, &triangleStartIndices );

    const std::vector<unsigned int>& indices  = surface->triangleIndices();
    const std::vector<cvf::Vec3d>&   vertices = surface->vertices();

    if ( !triangleStartIndices.empty() )
    {
        for ( auto triangleStartIndex : triangleStartIndices )
        {
            bool isLineDirDotNormalNegative = false;
            if ( cvf::GeometryTools::intersectLineSegmentTriangle( pointAbove,
                                                                   pointBelow,
                                                                   vertices[indices[triangleStartIndex + 0]],
                                                                   vertices[indices[triangleStartIndex + 1]],
                                                                   vertices[indices[triangleStartIndex + 2]],
                                                                   &intersectionPoint,
                                                                   &isLineDirDotNormalNegative ) == 1 )
                return true;
        }
    }

    // Handle cases where no match is found due to floating point imprecision,
    // or when falling off resulting grid slightly.
    // Use the XY extent of a triangle to define a suitable search distance
    double maxDistance = 10.0;
    {
        auto maxX = surface->maxExtentTriangleInXDirection() / 2.0;
        auto maxY = surface->maxExtentTriangleInYDirection() / 2.0;

        auto candidate = std::min( maxX, maxY );

        maxDistance = std::max( maxDistance, candidate );
    }

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
