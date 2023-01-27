/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022 -     Equinor ASA
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

#include "RigConvexHull.h"

#include <algorithm>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RigConvexHull::compute2d( const std::vector<cvf::Vec3d>& points )
{
    // Sort our points from left to right
    std::vector<cvf::Vec3d> sortedPoints = sortPoints( points );

    // Find the lower half of the convex hull.
    std::vector<cvf::Vec3d> lower;
    for ( auto it = sortedPoints.begin(); it != sortedPoints.end(); ++it )
    {
        // Remove any points that does not make a convex angle with the current point
        removePointsWithoutConvexAngle( lower, *it );
        lower.push_back( *it );
    }

    // Find the upper half of the convex hull.
    std::vector<cvf::Vec3d> upper;
    for ( auto it = sortedPoints.rbegin(); it != sortedPoints.rend(); ++it )
    {
        // Remove any points that does not make a convex angle with the current point
        removePointsWithoutConvexAngle( upper, *it );
        upper.push_back( *it );
    }

    // Concatenation of the lower and upper hulls gives the convex hull.
    // Last point of each list is omitted because it is repeated at the beginning of the other list.
    std::vector<cvf::Vec3d> hull;
    hull.insert( hull.end(), lower.begin(), lower.end() - 1 );
    hull.insert( hull.end(), upper.begin(), upper.end() - 1 );
    return hull;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RigConvexHull::sortPoints( const std::vector<cvf::Vec3d>& unsorted )
{
    // Returns true if a is left of b
    auto isLeftOf = []( const cvf::Vec3d& a, const cvf::Vec3d& b ) { return ( a.x() < b.x() || ( a.x() == b.x() && a.y() < b.y() ) ); };

    std::vector<cvf::Vec3d> sorted = unsorted;
    std::sort( sorted.begin(), sorted.end(), isLeftOf );
    return sorted;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigConvexHull::removePointsWithoutConvexAngle( std::vector<cvf::Vec3d>& points, const cvf::Vec3d& current )
{
    // 2D cross product of (a, b) and (a, c) vectors, i.e. z-component of their 3D cross product.
    // Returns a positive value, if c->(a,b) makes a counter-clockwise turn,
    // negative for clockwise turn, and zero if the points are collinear.
    auto counterClockWise = []( const cvf::Vec3d& a, const cvf::Vec3d& b, const cvf::Vec3d& c ) {
        return ( b.x() - a.x() ) * ( c.y() - a.y() ) - ( b.y() - a.y() ) * ( c.x() - a.x() );
    };

    // Remove all points that is not a counter-clockwise turn from current point
    while ( points.size() >= 2 && counterClockWise( *( points.rbegin() + 1 ), *( points.rbegin() ), current ) >= 0 )
    {
        points.pop_back();
    }
}
