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

    std::set<HexIntersectionInfo> uniqueIntersections;

    for ( int face = 0; face < 6; ++face )
    {
        cvf::ubyte faceVertexIndices[4];
        cvf::StructGridInterface::cellFaceVertexIndices( static_cast<cvf::StructGridInterface::FaceType>( face ), faceVertexIndices );

        cvf::Vec3d intersection;
        bool       isEntering = false;
        cvf::Vec3d faceCenter = cvf::GeometryTools::computeFaceCenter( hexCorners[faceVertexIndices[0]],
                                                                       hexCorners[faceVertexIndices[1]],
                                                                       hexCorners[faceVertexIndices[2]],
                                                                       hexCorners[faceVertexIndices[3]] );

        for ( int i = 0; i < 4; ++i )
        {
            int next = i < 3 ? i + 1 : 0;

            int intsStatus = cvf::GeometryTools::intersectLineSegmentTriangle( p1,
                                                                               p2,
                                                                               hexCorners[faceVertexIndices[i]],
                                                                               hexCorners[faceVertexIndices[next]],
                                                                               faceCenter,
                                                                               &intersection,
                                                                               &isEntering,
                                                                               parametricTolerance );
            if ( intsStatus == 1 )
            {
                uniqueIntersections.insert(
                    HexIntersectionInfo( intersection, isEntering, static_cast<cvf::StructGridInterface::FaceType>( face ), hexIndex ) );
            }
        }
    }

    int intersectionCount = 0;
    for ( const auto& intersection : uniqueIntersections )
    {
        intersections->push_back( intersection );
        ++intersectionCount;
    }

    return intersectionCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigHexIntersectionTools::lineIntersectsHexCell( const cvf::Vec3d& p1, const cvf::Vec3d& p2, const std::array<cvf::Vec3d, 8>& hexCorners )
{
    for ( int face = 0; face < 6; ++face )
    {
        cvf::ubyte faceVertexIndices[4];
        cvf::StructGridInterface::cellFaceVertexIndices( static_cast<cvf::StructGridInterface::FaceType>( face ), faceVertexIndices );

        cvf::Vec3d intersection;
        bool       isEntering = false;
        cvf::Vec3d faceCenter = cvf::GeometryTools::computeFaceCenter( hexCorners[faceVertexIndices[0]],
                                                                       hexCorners[faceVertexIndices[1]],
                                                                       hexCorners[faceVertexIndices[2]],
                                                                       hexCorners[faceVertexIndices[3]] );

        for ( int i = 0; i < 4; ++i )
        {
            int next = i < 3 ? i + 1 : 0;

            int intsStatus = cvf::GeometryTools::intersectLineSegmentTriangle( p1,
                                                                               p2,
                                                                               hexCorners[faceVertexIndices[i]],
                                                                               hexCorners[faceVertexIndices[next]],
                                                                               faceCenter,
                                                                               &intersection,
                                                                               &isEntering );
            if ( intsStatus == 1 )
            {
                return true;
            }
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// Casts a ray downwards from the point and counts the crossings of the cell surface. The faces are split into four triangles
/// around the face centre, so a ray through a face centre, a triangle edge or a cell edge hits several triangles at the same
/// point. These hits are one crossing of the surface, and are counted once.
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
    for ( size_t i = 0; i < 8; ++i )
    {
        corners[i] = hexCorners[i] - point;
        minX       = std::min( minX, corners[i].x() );
        maxX       = std::max( maxX, corners[i].x() );
        minY       = std::min( minY, corners[i].y() );
        maxY       = std::max( maxY, corners[i].y() );
        minZ       = std::min( minZ, corners[i].z() );
    }
    if ( minX > 0.0 || maxX < 0.0 || minY > 0.0 || maxY < 0.0 || minZ >= 0.0 ) return false;

    // Barycentric tolerance accepting hits on the edges shared by two triangles, which the exact test can miss on both triangles
    const double edgeTolerance = 1e-9;

    std::array<double, 24> hitDepths;
    size_t                 hitCount = 0;

    // The ray is vertical, so a triangle is hit when the point is inside the triangle projected to the XY plane, and the
    // triangle is below the point. Vertical triangles are parallel to the ray and are not hit.
    auto addHit = [&]( const cvf::Vec3d& a, const cvf::Vec3d& b, const cvf::Vec3d& c )
    {
        const double det = ( b.x() - a.x() ) * ( c.y() - a.y() ) - ( c.x() - a.x() ) * ( b.y() - a.y() );
        if ( std::abs( det ) < 1e-12 ) return;

        const double wa = ( b.x() * c.y() - c.x() * b.y() ) / det;
        const double wb = ( c.x() * a.y() - a.x() * c.y() ) / det;
        const double wc = 1.0 - wa - wb;
        if ( wa < -edgeTolerance || wb < -edgeTolerance || wc < -edgeTolerance ) return;

        const double z = wa * a.z() + wb * b.z() + wc * c.z();
        if ( z <= 0.0 ) hitDepths[hitCount++] = z;
    };

    for ( int face = 0; face < 6; ++face )
    {
        cvf::ubyte faceVertexIndices[4];
        cvf::StructGridInterface::cellFaceVertexIndices( static_cast<cvf::StructGridInterface::FaceType>( face ), faceVertexIndices );
        const cvf::Vec3d faceCenter = cvf::GeometryTools::computeFaceCenter( corners[faceVertexIndices[0]],
                                                                             corners[faceVertexIndices[1]],
                                                                             corners[faceVertexIndices[2]],
                                                                             corners[faceVertexIndices[3]] );

        for ( int i = 0; i < 4; ++i )
        {
            const int next = i < 3 ? i + 1 : 0;
            addHit( corners[faceVertexIndices[i]], corners[faceVertexIndices[next]], faceCenter );
        }
    }

    std::sort( hitDepths.begin(), hitDepths.begin() + hitCount );

    const double depthTolerance = 1e-6;
    size_t       crossings      = 0;
    for ( size_t i = 0; i < hitCount; ++i )
    {
        if ( i == 0 || hitDepths[i] - hitDepths[i - 1] > depthTolerance ) ++crossings;
    }

    return crossings % 2 == 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigHexIntersectionTools::planeHexCellIntersection( const std::array<cvf::Vec3d, 8>&              hexCorners,
                                                        const cvf::Plane&                             fracturePlane,
                                                        std::list<std::pair<cvf::Vec3d, cvf::Vec3d>>& intersectionLineSegments )
{
    bool                                  isCellIntersected = false;
    cvf::ubyte                            faceVertexIndices[4];
    caf::HexGridIntersectionTools::ClipVx triangleIntersectionPoint1;
    caf::HexGridIntersectionTools::ClipVx triangleIntersectionPoint2;
    bool                                  isMostVxesOnPositiveSideOfP1 = false;

    for ( int face = 0; face < 6; ++face )
    {
        cvf::StructGridInterface::cellFaceVertexIndices( static_cast<cvf::StructGridInterface::FaceType>( face ), faceVertexIndices );

        cvf::Vec3d faceCenter = cvf::GeometryTools::computeFaceCenter( hexCorners[faceVertexIndices[0]],
                                                                       hexCorners[faceVertexIndices[1]],
                                                                       hexCorners[faceVertexIndices[2]],
                                                                       hexCorners[faceVertexIndices[3]] );

        for ( int i = 0; i < 4; i++ )
        {
            int next = i < 3 ? i + 1 : 0;

            bool isIntersectingPlane = caf::HexGridIntersectionTools::planeTriangleIntersection( fracturePlane,
                                                                                                 hexCorners[faceVertexIndices[i]],
                                                                                                 0,
                                                                                                 hexCorners[faceVertexIndices[next]],
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
        }
    }
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
    const double tolerance = 1e-6;

    if ( hi1.m_hexIndex != hi2.m_hexIndex ) return hi1.m_hexIndex < hi2.m_hexIndex;
    if ( hi1.m_face != hi2.m_face ) return hi1.m_face < hi2.m_face;
    if ( hi1.m_isIntersectionEntering != hi2.m_isIntersectionEntering ) return hi1.m_isIntersectionEntering < hi2.m_isIntersectionEntering;
    if ( hi1.m_face != hi2.m_face ) return hi1.m_face < hi2.m_face;

    if ( cvf::Math::abs( hi2.m_intersectionPoint.x() - hi1.m_intersectionPoint.x() ) > tolerance )
        return hi1.m_intersectionPoint.x() < hi2.m_intersectionPoint.x();
    if ( cvf::Math::abs( hi2.m_intersectionPoint.y() - hi1.m_intersectionPoint.y() ) > tolerance )
        return hi1.m_intersectionPoint.y() < hi2.m_intersectionPoint.y();
    if ( cvf::Math::abs( hi2.m_intersectionPoint.z() - hi1.m_intersectionPoint.z() ) > tolerance )
        return hi1.m_intersectionPoint.z() < hi2.m_intersectionPoint.z();

    return false;
}
