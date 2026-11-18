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

//--------------------------------------------------------------------------------------------------
/// Conservative tests used to skip the triangle tests: a test returns false only if the segment is outside the
/// bounding box of the cell or of a cell face. A reported intersection point is on the segment and inside a surface
/// triangle within the parametric tolerance, so the boxes are extended by a margin covering that tolerance and the
/// floating point resolution.
//--------------------------------------------------------------------------------------------------
class SegmentBoxTest
{
public:
    SegmentBoxTest( const cvf::Vec3d& p1, const cvf::Vec3d& p2, const std::array<cvf::Vec3d, 8>& hexCorners, double parametricTolerance )
        : m_p1( p1 )
        , m_halfDirection( ( p2 - p1 ) * 0.5 )
        , m_absHalfDirection( std::abs( m_halfDirection.x() ), std::abs( m_halfDirection.y() ), std::abs( m_halfDirection.z() ) )
        , m_cellMin( hexCorners[0] )
        , m_cellMax( hexCorners[0] )
    {
        for ( size_t i = 1; i < 8; ++i )
        {
            extendBox( hexCorners[i], m_cellMin, m_cellMax );
        }

        double scale               = 0.0;
        double coordinateMagnitude = 0.0;
        for ( int c = 0; c < 3; ++c )
        {
            scale = std::max( { scale, 0.5 * ( m_cellMax[c] - m_cellMin[c] ), m_absHalfDirection[c] } );
            coordinateMagnitude =
                std::max( { coordinateMagnitude, std::abs( m_cellMin[c] ), std::abs( m_cellMax[c] ), std::abs( p1[c] ), std::abs( p2[c] ) } );
        }
        m_margin = 2.0 * ( 1.0e-6 + 8.0 * parametricTolerance ) * scale + 1.0e-12 * coordinateMagnitude;
    }

    bool canIntersectCell() const { return canIntersectBox( m_cellMin, m_cellMax ); }

    // The face center is the average of the corners, so the triangles of the face are inside the box of the corners
    bool canIntersectFace( const cvf::Vec3d& v0, const cvf::Vec3d& v1, const cvf::Vec3d& v2, const cvf::Vec3d& v3 ) const
    {
        cvf::Vec3d minCorner = v0;
        cvf::Vec3d maxCorner = v0;
        extendBox( v1, minCorner, maxCorner );
        extendBox( v2, minCorner, maxCorner );
        extendBox( v3, minCorner, maxCorner );

        return canIntersectBox( minCorner, maxCorner );
    }

private:
    static void extendBox( const cvf::Vec3d& point, cvf::Vec3d& minCorner, cvf::Vec3d& maxCorner )
    {
        for ( int c = 0; c < 3; ++c )
        {
            minCorner[c] = std::min( minCorner[c], point[c] );
            maxCorner[c] = std::max( maxCorner[c], point[c] );
        }
    }

    // Separating axis test of the segment and the extended box, with the segment relative to the box center
    bool canIntersectBox( const cvf::Vec3d& minCorner, const cvf::Vec3d& maxCorner ) const
    {
        const cvf::Vec3d  halfExtent = ( maxCorner - minCorner ) * 0.5;
        const cvf::Vec3d  m          = ( m_p1 - minCorner ) - halfExtent + m_halfDirection;
        const cvf::Vec3d  e          = halfExtent + cvf::Vec3d( m_margin, m_margin, m_margin );
        const cvf::Vec3d& d          = m_absHalfDirection;

        if ( std::abs( m.x() ) > e.x() + d.x() ) return false;
        if ( std::abs( m.y() ) > e.y() + d.y() ) return false;
        if ( std::abs( m.z() ) > e.z() + d.z() ) return false;

        // The cross product axes are not normalized, so the margin is included in the extents on the right hand side
        if ( std::abs( m.y() * m_halfDirection.z() - m.z() * m_halfDirection.y() ) > e.y() * d.z() + e.z() * d.y() ) return false;
        if ( std::abs( m.z() * m_halfDirection.x() - m.x() * m_halfDirection.z() ) > e.z() * d.x() + e.x() * d.z() ) return false;
        if ( std::abs( m.x() * m_halfDirection.y() - m.y() * m_halfDirection.x() ) > e.x() * d.y() + e.y() * d.x() ) return false;

        return true;
    }

private:
    cvf::Vec3d m_p1;
    cvf::Vec3d m_halfDirection;
    cvf::Vec3d m_absHalfDirection;
    cvf::Vec3d m_cellMin;
    cvf::Vec3d m_cellMax;
    double     m_margin = 0.0;
};

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

    const SegmentBoxTest boxTest( p1, p2, hexCorners, parametricTolerance );
    if ( !boxTest.canIntersectCell() ) return 0;

    const auto canIntersectFace = [&]( const cvf::Vec3d& v0, const cvf::Vec3d& v1, const cvf::Vec3d& v2, const cvf::Vec3d& v3 )
    { return boxTest.canIntersectFace( v0, v1, v2, v3 ); };

    const cvf::Vec3d direction     = p2 - p1;
    const double     lengthSquared = direction.lengthSquared();

    // At most one hit per surface triangle
    std::array<SegmentHit, 24> hits;
    size_t                     hitCount = 0;

    forEachCellSurfaceTriangle( hexCorners,
                                canIntersectFace,
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
    const SegmentBoxTest boxTest( p1, p2, hexCorners, 0.0 );
    if ( !boxTest.canIntersectCell() ) return false;

    const bool allVisited = forEachCellSurfaceTriangle(
        hexCorners,
        [&]( const cvf::Vec3d& v0, const cvf::Vec3d& v1, const cvf::Vec3d& v2, const cvf::Vec3d& v3 )
        { return boxTest.canIntersectFace( v0, v1, v2, v3 ); },
        [&]( cvf::StructGridInterface::FaceType, const cvf::Vec3d& t0, const cvf::Vec3d& t1, const cvf::Vec3d& t2 )
        {
            cvf::Vec3d intersection;
            bool       isEntering = false;
            return cvf::GeometryTools::intersectLineSegmentTriangle( p1, p2, t0, t1, t2, &intersection, &isEntering ) != 1;
        } );
    return !allVisited;
}

//--------------------------------------------------------------------------------------------------
/// Casts a ray downwards from the point and counts the crossings of the cell surface. The faces are split into four triangles
/// around the face centre, so a ray through a face centre, a triangle edge or a cell edge hits several triangles at the same
/// point. Hits at the same point are one crossing of the surface if the triangles face the same way along the ray. If they
/// face opposite ways, the ray only touches the surface, for instance at a ridge of the cell, and the hits cancel.
//--------------------------------------------------------------------------------------------------
bool RigHexIntersectionTools::isPointInCell( const cvf::Vec3d& point, const std::array<cvf::Vec3d, 8>& hexCorners )
{
    // Corners relative to the point, to keep the precision at UTM coordinates
    std::array<cvf::Vec3d, 8> corners;
    double                    minX = std::numeric_limits<double>::max();
    double                    maxX = -minX;
    double                    minY = minX;
    double                    maxY = -minX;
    double                    minZ = minX;
    double                    maxZ = -minX;
    for ( size_t i = 0; i < 8; ++i )
    {
        corners[i] = hexCorners[i] - point;
        minX       = std::min( minX, corners[i].x() );
        maxX       = std::max( maxX, corners[i].x() );
        minY       = std::min( minY, corners[i].y() );
        maxY       = std::max( maxY, corners[i].y() );
        minZ       = std::min( minZ, corners[i].z() );
        maxZ       = std::max( maxZ, corners[i].z() );
    }
    if ( minX > 0.0 || maxX < 0.0 || minY > 0.0 || maxY < 0.0 || minZ > 0.0 ) return false;

    // The point is inside the bounding box in X and Y, so the extent from the point is a measure of the cell size
    const double horizontalScale = std::max( { -minX, maxX, -minY, maxY } );
    const double scale           = std::max( { horizontalScale, -minZ, maxZ } );

    // Barycentric tolerance accepting hits on the edges shared by two triangles, which the exact test can miss on both triangles
    const double edgeTolerance = 1e-9;

    // Tolerances relative to the size of the cell, as the point is inside the bounding box of the cell. The degenerate
    // tolerance is compared to an area in the XY plane, so it is not scaled by the vertical extent of the cell.
    const double degenerateTolerance = 1e-14 * horizontalScale * horizontalScale;
    const double depthTolerance      = 1e-8 * scale;

    struct Hit
    {
        double depth;
        int    direction; // +1 if the triangle faces up, -1 if it faces down. The faces of a cell are oriented consistently.
    };
    std::array<Hit, 24> hits;
    size_t              hitCount = 0;

    // The ray is vertical, so a triangle is hit when the point is inside the triangle projected to the XY plane, and the
    // triangle is below the point. Vertical triangles are parallel to the ray and are not hit.
    auto addHit = [&]( const cvf::Vec3d& a, const cvf::Vec3d& b, const cvf::Vec3d& c )
    {
        // Most triangles are on one side of the point in X or Y
        if ( ( a.x() > 0.0 && b.x() > 0.0 && c.x() > 0.0 ) || ( a.x() < 0.0 && b.x() < 0.0 && c.x() < 0.0 ) ||
             ( a.y() > 0.0 && b.y() > 0.0 && c.y() > 0.0 ) || ( a.y() < 0.0 && b.y() < 0.0 && c.y() < 0.0 ) )
            return;

        // The Z component of the triangle normal, which is twice the signed area of the triangle projected to the XY plane
        const double det    = ( b.x() - a.x() ) * ( c.y() - a.y() ) - ( c.x() - a.x() ) * ( b.y() - a.y() );
        const double absDet = std::abs( det );
        if ( absDet <= degenerateTolerance ) return;

        // Barycentric coordinates scaled by det, to reject most triangles without dividing
        const double sign = det > 0.0 ? 1.0 : -1.0;
        const double na   = sign * ( b.x() * c.y() - c.x() * b.y() );
        const double nb   = sign * ( c.x() * a.y() - a.x() * c.y() );
        const double nc   = absDet - na - nb;
        const double tol  = -edgeTolerance * absDet;
        if ( na < tol || nb < tol || nc < tol ) return;

        const double z = ( na * a.z() + nb * b.z() + nc * c.z() ) / absDet;
        if ( z <= 0.0 )
        {
            // Insertion sort by depth, as there are few hits
            size_t pos = hitCount++;
            for ( ; pos > 0 && hits[pos - 1].depth > z; --pos )
                hits[pos] = hits[pos - 1];
            hits[pos] = { z, det > 0.0 ? 1 : -1 };
        }
    };

    forEachCellSurfaceTriangle( corners,
                                [&]( cvf::StructGridInterface::FaceType, const cvf::Vec3d& t0, const cvf::Vec3d& t1, const cvf::Vec3d& t2 )
                                {
                                    addHit( t0, t1, t2 );
                                    return true;
                                } );

    // Hits at the same depth (within tolerance) are the same surface crossing, seen through triangles that share an edge
    // or a face. Count each direction once per group of hits. The sum is zero outside the closed surface, and plus or
    // minus one inside, depending on the handedness of the grid.
    int    crossings          = 0;
    double groupDepth         = 0.0;
    bool   groupHasUpFacing   = false;
    bool   groupHasDownFacing = false;

    for ( size_t i = 0; i < hitCount; ++i )
    {
        const bool startsNewGroup = ( i == 0 ) || ( hits[i].depth - groupDepth > depthTolerance );
        if ( startsNewGroup )
        {
            groupDepth         = hits[i].depth;
            groupHasUpFacing   = false;
            groupHasDownFacing = false;
        }

        if ( hits[i].direction > 0 )
        {
            if ( !groupHasUpFacing ) crossings++;
            groupHasUpFacing = true;
        }
        else
        {
            if ( !groupHasDownFacing ) crossings--;
            groupHasDownFacing = true;
        }
    }

    return crossings != 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigHexIntersectionTools::planeHexCellIntersection( const std::array<cvf::Vec3d, 8>&              hexCorners,
                                                        const cvf::Plane&                             plane,
                                                        std::list<std::pair<cvf::Vec3d, cvf::Vec3d>>& intersectionLineSegments )
{
    bool                                  isCellIntersected = false;
    caf::HexGridIntersectionTools::ClipVx triangleIntersectionPoint1;
    caf::HexGridIntersectionTools::ClipVx triangleIntersectionPoint2;
    bool                                  isMostVxesOnPositiveSideOfP1 = false;

    // The plane side of each corner and face center, evaluated as in planeTriangleIntersection(). A triangle with
    // all vertices on the same side is not intersected, so the triangle test is skipped.
    std::array<bool, 8> isCornerOnPositiveSide;
    for ( size_t i = 0; i < 8; ++i )
    {
        isCornerOnPositiveSide[i] = plane.distanceSquared( hexCorners[i] ) >= 0.0;
    }

    cvf::StructGridInterface::FaceType currentFace            = cvf::StructGridInterface::NO_FACE;
    bool                               isCenterOnPositiveSide = false;

    forEachCellSurfaceTriangleByCornerIndex(
        hexCorners,
        []( const cvf::Vec3d&, const cvf::Vec3d&, const cvf::Vec3d&, const cvf::Vec3d& ) { return true; },
        [&]( cvf::StructGridInterface::FaceType face, cvf::ubyte cornerIdx, cvf::ubyte nextCornerIdx, const cvf::Vec3d& faceCenter )
        {
            if ( face != currentFace )
            {
                currentFace            = face;
                isCenterOnPositiveSide = plane.distanceSquared( faceCenter ) >= 0.0;
            }

            const bool isCornerPositive     = isCornerOnPositiveSide[cornerIdx];
            const bool isNextCornerPositive = isCornerOnPositiveSide[nextCornerIdx];
            if ( isCornerPositive == isNextCornerPositive && isNextCornerPositive == isCenterOnPositiveSide ) return true;

            bool isIntersectingPlane = caf::HexGridIntersectionTools::planeTriangleIntersection( plane,
                                                                                                 hexCorners[cornerIdx],
                                                                                                 0,
                                                                                                 hexCorners[nextCornerIdx],
                                                                                                 1,
                                                                                                 faceCenter,
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

    cvf::Plane plane;
    plane.setFromPointAndNormal( transformMatrixForPlane.translation(), static_cast<cvf::Vec3d>( transformMatrixForPlane.col( 2 ) ) );

    // Find line-segments where cell and fracture plane intersects
    std::list<std::pair<cvf::Vec3d, cvf::Vec3d>> intersectionLineSegments;

    isCellIntersected = planeHexCellIntersection( hexCorners, plane, intersectionLineSegments );

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
