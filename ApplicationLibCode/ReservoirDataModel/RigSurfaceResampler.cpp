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
#include "Well/RigWellPath.h"

#include "cvfGeometryTools.h"

#include "cvfBoundingBox.h"
#include "cvfObject.h"

#include <limits>

//--------------------------------------------------------------------------------------------------
/// Create a surface with same XY coordinates as targetSurface. Evaluate Z value at [X,Y] in the source surface. Search in XY plane to find
/// closest point, limited by a max distance from [X,Y].
//--------------------------------------------------------------------------------------------------
cvf::ref<RigSurface> RigSurfaceResampler::resampleSurface( cvf::ref<RigSurface> targetSurface, cvf::ref<RigSurface> surface )
{
    cvf::ref<RigSurface> resampledSurface = cvf::make_ref<RigSurface>();

    const std::vector<cvf::Vec3d>& targetVerts   = targetSurface->vertices();
    const std::vector<unsigned>&   targetIndices = targetSurface->triangleIndices();

    std::vector<cvf::Vec3d> resampledVerts;

    for ( const auto& targetVert : targetVerts )
    {
        const cvf::Vec3d pointAbove = cvf::Vec3d( targetVert.x(), targetVert.y(), 10000.0 );
        const cvf::Vec3d pointBelow = cvf::Vec3d( targetVert.x(), targetVert.y(), -10000.0 );

        cvf::Vec3d intersectionPoint;
        bool       foundMatch = findClosestPointOnSurface( surface.p(), pointAbove, pointBelow, intersectionPoint );
        if ( !foundMatch ) intersectionPoint = cvf::Vec3d( targetVert.x(), targetVert.y(), std::numeric_limits<double>::infinity() );

        resampledVerts.push_back( intersectionPoint );
    }

    resampledSurface->setTriangleData( targetIndices, resampledVerts );

    return resampledSurface;
}

//--------------------------------------------------------------------------------------------------
/// If an intersection point is found, return true and set intersectionPoint to the intersection point.
/// If no intersection point is found, return false.
//--------------------------------------------------------------------------------------------------
bool RigSurfaceResampler::computeIntersectionWithLine( RigSurface* surface, const cvf::Vec3d& p1, const cvf::Vec3d& p2, cvf::Vec3d& intersectionPoint )
{
    if ( !surface ) return false;

    surface->ensureIntersectionSearchTreeIsBuilt();

    cvf::BoundingBox bb;
    bb.add( p1 );
    bb.add( p2 );

    const std::vector<unsigned int>& triIndices = surface->triangleIndices();
    const std::vector<cvf::Vec3d>&   vertices   = surface->vertices();

    bool dummy = false;

    std::vector<size_t> triangleStartIndices;
    surface->findIntersectingTriangles( bb, &triangleStartIndices );

    for ( auto startIndex : triangleStartIndices )
    {
        if ( cvf::GeometryTools::intersectLineSegmentTriangle( p1,
                                                               p2,
                                                               vertices[triIndices[startIndex + 0]],
                                                               vertices[triIndices[startIndex + 1]],
                                                               vertices[triIndices[startIndex + 2]],
                                                               &intersectionPoint,
                                                               &dummy ) == 1 )
            return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// Find the closest point on the surface to the line. The search is limited to a max distance from the line based on the resolution
/// of the surface.
//--------------------------------------------------------------------------------------------------
bool RigSurfaceResampler::findClosestPointOnSurface( RigSurface* surface, const cvf::Vec3d& p1, const cvf::Vec3d& p2, cvf::Vec3d& intersectionPoint )
{
    if ( computeIntersectionWithLine( surface, p1, p2, intersectionPoint ) ) return true;

    cvf::BoundingBox bb;
    bb.add( p1 );
    bb.add( p2 );

    const std::vector<unsigned int>& triIndices = surface->triangleIndices();
    const std::vector<cvf::Vec3d>&   vertices   = surface->vertices();

    double maxDistance = computeMaxDistance( surface );

    // Expand the bounding box to cover a larger volume around bounding box of the line segment.
    bb.expand( maxDistance );

    std::vector<size_t> triangleStartIndices;
    surface->findIntersectingTriangles( bb, &triangleStartIndices );

    return findClosestPointXY( p1, vertices, triIndices, triangleStartIndices, maxDistance, intersectionPoint );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RigSurfaceResampler::computeResampledPolyline( const std::vector<cvf::Vec3d>& polyline, double resamplingDistance )
{
    auto polylineAndSegmentData = computeResampledPolylineWithSegmentInfoImpl( polyline, resamplingDistance );

    return polylineAndSegmentData.first;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<cvf::Vec3d, size_t>>
    RigSurfaceResampler::computeResampledPolylineWithSegmentInfo( const std::vector<cvf::Vec3d>& polyline, double resamplingDistance )
{
    // Segments along the original polyline must be provided to be able to find the associated transform matrix

    std::vector<cvf::Vec3d> resampledPolyline;
    std::vector<size_t>     segmentIndices;

    std::tie( resampledPolyline, segmentIndices ) = computeResampledPolylineWithSegmentInfoImpl( polyline, resamplingDistance );

    if ( resampledPolyline.size() != segmentIndices.size() ) return {};

    // Create vector of pairs based on pair of two vectors

    std::vector<std::pair<cvf::Vec3d, size_t>> polyLineAndSegment;
    for ( size_t i = 0; i < resampledPolyline.size(); i++ )
    {
        polyLineAndSegment.push_back( std::make_pair( resampledPolyline[i], segmentIndices[i] ) );
    }

    return polyLineAndSegment;
}

//--------------------------------------------------------------------------------------------------
/// Find the closest point in XY plane closer than maxDistance
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<cvf::Vec3d>, std::vector<size_t>>
    RigSurfaceResampler::computeResampledPolylineWithSegmentInfoImpl( const std::vector<cvf::Vec3d>& polyline, double resamplingDistance )
{
    // Segments along the original polyline must be provided to be able to find the associated transform matrix
    std::vector<cvf::Vec3d> resampledPolyline;
    std::vector<size_t>     segmentIndices;

    if ( polyline.size() > 1 )
    {
        std::vector<double> measuredDepth;
        {
            double aggregatedLength = 0.0;

            cvf::Vec3d previousPoint = polyline.front();
            measuredDepth.push_back( aggregatedLength );

            for ( size_t i = 1; i < polyline.size(); i++ )
            {
                aggregatedLength += ( previousPoint - polyline[i] ).length();
                previousPoint = polyline[i];
                measuredDepth.push_back( aggregatedLength );
            }
        }

        // Use RigWellPath to perform the interpolation along a line based on measured depth
        RigWellPath dummyWellPath( polyline, measuredDepth );

        for ( size_t i = 1; i < polyline.size(); i++ )
        {
            const auto& lineSegmentStart = polyline[i - 1];
            const auto& lineSegmentEnd   = polyline[i];

            auto startMD = measuredDepth[i - 1];
            auto endMD   = measuredDepth[i];

            const size_t segmentIndex = i - 1;
            resampledPolyline.emplace_back( lineSegmentStart );
            segmentIndices.emplace_back( segmentIndex );

            for ( auto md = startMD + resamplingDistance; md < endMD; md += resamplingDistance )
            {
                resampledPolyline.emplace_back( dummyWellPath.interpolatedPointAlongWellPath( md ) );
                segmentIndices.emplace_back( segmentIndex );
            }

            resampledPolyline.emplace_back( lineSegmentEnd );
            segmentIndices.emplace_back( segmentIndex );
        }
    }

    return std::make_pair( resampledPolyline, segmentIndices );
}
