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

    const std::vector<unsigned int>& triIndices = surface->triangleIndices();
    const std::vector<cvf::Vec3d>&   vertices   = surface->vertices();

    {
        std::vector<size_t> triangleStartIndices;
        surface->findIntersectingTriangles( bb, &triangleStartIndices );

        if ( !triangleStartIndices.empty() )
        {
            for ( auto triangleStartIndex : triangleStartIndices )
            {
                bool isLineDirDotNormalNegative = false;
                if ( cvf::GeometryTools::intersectLineSegmentTriangle( pointAbove,
                                                                       pointBelow,
                                                                       vertices[triIndices[triangleStartIndex + 0]],
                                                                       vertices[triIndices[triangleStartIndex + 1]],
                                                                       vertices[triIndices[triangleStartIndex + 2]],
                                                                       &intersectionPoint,
                                                                       &isLineDirDotNormalNegative ) == 1 )
                    return true;
            }
        }
    }

    double maxDistance = computeMaxDistance( surface );

    // Expand the bounding box to cover a larger volume. Use this volume to find intersections.
    bb.expand( maxDistance );

    std::vector<size_t> triangleStartIndices;
    surface->findIntersectingTriangles( bb, &triangleStartIndices );

    return findClosestPointXY( pointAbove, vertices, triIndices, triangleStartIndices, maxDistance, intersectionPoint );
}

//--------------------------------------------------------------------------------------------------
/// Find the closest vertex to targetPoint (must be closer than maxDistance) in XY plane.
/// Unit maxDistance: meter.
//--------------------------------------------------------------------------------------------------
bool RigSurfaceResampler::findClosestPointXY( const cvf::Vec3d&                targetPoint,
                                              const std::vector<cvf::Vec3d>&   vertices,
                                              const std::vector<unsigned int>& triangleIndices,
                                              const std::vector<size_t>&       triangleStartIndices,
                                              double                           maxDistance,
                                              cvf::Vec3d&                      intersectionPoint )
{
    double maxDistanceSquared = maxDistance * maxDistance;

    // Find closest vertices
    double     shortestDistanceSquared = std::numeric_limits<double>::max();
    double     closestZ                = std::numeric_limits<double>::infinity();
    cvf::Vec3d p;
    double     distanceSquared = 0.0;
    for ( auto triangleStartIndex : triangleStartIndices )
    {
        for ( size_t localIdx = 0; localIdx < 3; localIdx++ )
        {
            const auto& v = vertices[triangleIndices[triangleStartIndex + localIdx]];

            if ( std::fabs( targetPoint.x() - v.x() ) > maxDistance ) continue;
            if ( std::fabs( targetPoint.y() - v.y() ) > maxDistance ) continue;

            // Ignore height (z) component when finding closest by
            // moving point to same height as target point above
            p.x() = v.x();
            p.y() = v.y();
            p.z() = targetPoint.z();

            distanceSquared = p.pointDistanceSquared( targetPoint );
            if ( distanceSquared < shortestDistanceSquared )
            {
                shortestDistanceSquared = distanceSquared;
                closestZ                = v.z();
            }
        }
    }

    // Check if the closest point is not to far away to be valid
    if ( shortestDistanceSquared < maxDistanceSquared )
    {
        intersectionPoint = cvf::Vec3d( targetPoint.x(), targetPoint.y(), closestZ );
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigSurfaceResampler::computeMaxDistance( RigSurface* surface )
{
    // Handle cases where no match is found due to floating point imprecision,
    // or when falling off resulting grid slightly.
    // Use the XY extent of a triangle to define a suitable search distance

    const double minimumDistance = 10.0;

    if ( !surface ) return minimumDistance;

    auto maxX = surface->maxExtentTriangleInXDirection() / 2.0;
    auto maxY = surface->maxExtentTriangleInYDirection() / 2.0;

    auto candidate = std::min( maxX, maxY );

    double distance = std::max( minimumDistance, candidate );

    return distance;
}
