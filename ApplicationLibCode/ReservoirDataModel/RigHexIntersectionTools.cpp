/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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
#include "RigHexIntersectionTools.h"

#include "RigCellGeometryTools.h"

#include "cvfBoundingBox.h"
#include "cvfGeometryTools.h"
#include "cvfRay.h"

#include "cafAssert.h"
#include "cafHexGridIntersectionTools/cafHexGridIntersectionTools.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool isLexicographicallyLess( const cvf::Vec3d& a, const cvf::Vec3d& b )
{
    if ( a.x() != b.x() ) return a.x() < b.x();
    if ( a.y() != b.y() ) return a.y() < b.y();
    return a.z() < b.z();
}

using SegmentHit = RigHexIntersectionTools::SegmentHit;

//--------------------------------------------------------------------------------------------------
/// Exact ordering on (t, entering, face, point)
//--------------------------------------------------------------------------------------------------
bool isHitLess( const SegmentHit& a, const SegmentHit& b )
{
    if ( a.t != b.t ) return a.t < b.t;
    if ( a.isEntering != b.isEntering ) return a.isEntering < b.isEntering;
    if ( a.face != b.face ) return a.face < b.face;
    return isLexicographicallyLess( a.point, b.point );
}

} // namespace

//--------------------------------------------------------------------------------------------------
/// Face corners are listed in cyclic order. Any rotation or reversal of that order keeps the two diagonals as the
/// same pairs of corners, and floating point addition is commutative, so summing the diagonals first gives a
/// bit-identical result for every listing of the same face.
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigHexIntersectionTools::canonicalFaceCenter( const cvf::Vec3d& v0, const cvf::Vec3d& v1, const cvf::Vec3d& v2, const cvf::Vec3d& v3 )
{
    return ( ( v0 + v2 ) + ( v1 + v3 ) ) * 0.25;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigHexIntersectionTools::lineHexCellIntersection( const cvf::Vec3d&                 p1,
                                                      const cvf::Vec3d&                 p2,
                                                      const std::array<cvf::Vec3d, 8>&  hexCorners,
                                                      const size_t                      hexIndex,
                                                      std::vector<HexIntersectionInfo>* intersections,
                                                      double                            parametricTolerance )
{
    CAF_ASSERT( intersections != nullptr );

    const cvf::Vec3d direction     = p2 - p1;
    const double     lengthSquared = direction.lengthSquared();

    // At most one hit per surface triangle
    std::array<SegmentHit, 24> hits;
    size_t                     hitCount = 0;

    forEachCellSurfaceTriangle( hexCorners,
                                [&]( cvf::StructGridInterface::FaceType face, const cvf::Vec3d& t0, const cvf::Vec3d& t1, const cvf::Vec3d& t2 )
                                {
                                    cvf::Vec3d intersection;
                                    bool       isEntering = false;

                                    int intsStatus = cvf::GeometryTools::intersectLineSegmentTriangle( p1,
                                                                                                       p2,
                                                                                                       t0,
                                                                                                       t1,
                                                                                                       t2,
                                                                                                       &intersection,
                                                                                                       &isEntering,
                                                                                                       parametricTolerance );
                                    if ( intsStatus == 1 )
                                    {
                                        const double t = lengthSquared > 0.0 ? ( intersection - p1 ).dot( direction ) / lengthSquared : 0.0;
                                        hits[hitCount++] = { t, intersection, isEntering, face };
                                    }
                                    return true;
                                } );

    if ( hitCount == 0 ) return 0;

    // Hits on a shared triangle or face edge are reported once per triangle. Merge hits in the same direction
    // that are closer than a tolerance relative to the segment length. The tolerance is never below what the
    // coordinate magnitude allows to resolve.
    double mergeTolerance = intersectionMergeTolerance;
    if ( hitCount > 1 && lengthSquared > 0.0 )
    {
        double coordinateMagnitude = 0.0;
        for ( int i = 0; i < 3; ++i )
        {
            coordinateMagnitude = std::max( { coordinateMagnitude, std::abs( p1[i] ), std::abs( p2[i] ) } );
        }
        const double resolvableTolerance = 64.0 * std::numeric_limits<double>::epsilon() * coordinateMagnitude / std::sqrt( lengthSquared );
        mergeTolerance                   = std::max( mergeTolerance, resolvableTolerance );
    }

    return sortMergeAndAppendHits( std::span<SegmentHit>( hits.data(), hitCount ), mergeTolerance, hexIndex, intersections );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigHexIntersectionTools::sortMergeAndAppendHits( std::span<SegmentHit>             hits,
                                                     double                            mergeTolerance,
                                                     size_t                            hexIndex,
                                                     std::vector<HexIntersectionInfo>* intersections )
{
    CAF_ASSERT( intersections != nullptr );

    // Insertion sort, as there are only a few hits
    for ( size_t i = 1; i < hits.size(); ++i )
    {
        SegmentHit hit = hits[i];
        size_t     j   = i;
        for ( ; j > 0 && isHitLess( hit, hits[j - 1] ); --j )
        {
            hits[j] = hits[j - 1];
        }
        hits[j] = hit;
    }

    // The last kept hit is the first hit of the current run, so comparing to it avoids chaining
    const SegmentHit* runStart = nullptr;

    int intersectionCount = 0;
    for ( const SegmentHit& hit : hits )
    {
        if ( runStart && runStart->isEntering == hit.isEntering && hit.t - runStart->t <= mergeTolerance ) continue;

        runStart = &hit;
        intersections->emplace_back( hit.point, hit.isEntering, hit.face, hexIndex );
        ++intersectionCount;
    }

    return intersectionCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigHexIntersectionTools::lineIntersectsHexCell( const cvf::Vec3d& p1, const cvf::Vec3d& p2, const std::array<cvf::Vec3d, 8>& hexCorners )
{
    const bool allVisited =
        forEachCellSurfaceTriangle( hexCorners,
                                    [&]( cvf::StructGridInterface::FaceType, const cvf::Vec3d& t0, const cvf::Vec3d& t1, const cvf::Vec3d& t2 )
                                    {
                                        cvf::Vec3d intersection;
                                        bool       isEntering = false;
                                        return cvf::GeometryTools::intersectLineSegmentTriangle( p1, p2, t0, t1, t2, &intersection, &isEntering ) !=
                                               1;
                                    } );
    return !allVisited;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigHexIntersectionTools::isPointInCell( const cvf::Vec3d& point, const std::array<cvf::Vec3d, 8>& hexCorners )
{
    cvf::Ray ray;
    ray.setOrigin( point );
    size_t intersections = 0;

    forEachCellSurfaceTriangle( hexCorners,
                                [&]( cvf::StructGridInterface::FaceType, const cvf::Vec3d& t0, const cvf::Vec3d& t1, const cvf::Vec3d& t2 )
                                {
                                    if ( ray.triangleIntersect( t0, t1, t2 ) ) ++intersections;
                                    return true;
                                } );

    return intersections % 2 == 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigHexIntersectionTools::planeHexCellIntersection( const std::array<cvf::Vec3d, 8>&              hexCorners,
                                                        const cvf::Plane&                             fracturePlane,
                                                        std::list<std::pair<cvf::Vec3d, cvf::Vec3d>>& intersectionLineSegments )
{
    bool                                  isCellIntersected = false;
    caf::HexGridIntersectionTools::ClipVx triangleIntersectionPoint1;
    caf::HexGridIntersectionTools::ClipVx triangleIntersectionPoint2;
    bool                                  isMostVxesOnPositiveSideOfP1 = false;

    forEachCellSurfaceTriangle( hexCorners,
                                [&]( cvf::StructGridInterface::FaceType, const cvf::Vec3d& t0, const cvf::Vec3d& t1, const cvf::Vec3d& t2 )
                                {
                                    bool isIntersectingPlane =
                                        caf::HexGridIntersectionTools::planeTriangleIntersection( fracturePlane,
                                                                                                  t0,
                                                                                                  0,
                                                                                                  t1,
                                                                                                  1,
                                                                                                  t2,
                                                                                                  2,
                                                                                                  &triangleIntersectionPoint1,
                                                                                                  &triangleIntersectionPoint2,
                                                                                                  &isMostVxesOnPositiveSideOfP1 );

                                    if ( isIntersectingPlane )
                                    {
                                        isCellIntersected = true;
                                        intersectionLineSegments.emplace_back( triangleIntersectionPoint1.vx, triangleIntersectionPoint2.vx );
                                    }
                                    return true;
                                } );

    return isCellIntersected;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigHexIntersectionTools::planeHexIntersectionPolygons( const std::array<cvf::Vec3d, 8>&      hexCorners,
                                                            cvf::Mat4d                            transformMatrixForPlane,
                                                            std::vector<std::vector<cvf::Vec3d>>& polygons )
{
    bool isCellIntersected = false;

    cvf::Plane fracturePlane;
    fracturePlane.setFromPointAndNormal( transformMatrixForPlane.translation(), static_cast<cvf::Vec3d>( transformMatrixForPlane.col( 2 ) ) );

    // Find line-segments where cell and fracture plane intersects
    std::list<std::pair<cvf::Vec3d, cvf::Vec3d>> intersectionLineSegments;

    isCellIntersected = planeHexCellIntersection( hexCorners, fracturePlane, intersectionLineSegments );

    RigCellGeometryTools::createPolygonFromLineSegments( intersectionLineSegments, polygons );

    return isCellIntersected;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool operator<( const HexIntersectionInfo& hi1, const HexIntersectionInfo& hi2 )
{
    if ( hi1.m_hexIndex != hi2.m_hexIndex ) return hi1.m_hexIndex < hi2.m_hexIndex;
    if ( hi1.m_face != hi2.m_face ) return hi1.m_face < hi2.m_face;
    if ( hi1.m_isIntersectionEntering != hi2.m_isIntersectionEntering ) return hi1.m_isIntersectionEntering < hi2.m_isIntersectionEntering;

    return isLexicographicallyLess( hi1.m_intersectionPoint, hi2.m_intersectionPoint );
}
