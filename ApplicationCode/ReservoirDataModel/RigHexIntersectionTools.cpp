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

#include "cafHexGridIntersectionTools/cafHexGridIntersectionTools.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RigHexIntersectionTools::lineHexCellIntersection(const cvf::Vec3d p1, 
                                                     const cvf::Vec3d p2, 
                                                     const cvf::Vec3d hexCorners[8], 
                                                     const size_t hexIndex, 
                                                     std::vector<HexIntersectionInfo>* intersections)
{
    CVF_ASSERT(intersections != NULL);

    std::set<HexIntersectionInfo> uniqueIntersections;

    for ( int face = 0; face < 6 ; ++face )
    {
        cvf::ubyte faceVertexIndices[4];
        cvf::StructGridInterface::cellFaceVertexIndices(static_cast<cvf::StructGridInterface::FaceType>(face), faceVertexIndices);

        cvf::Vec3d intersection;
        bool isEntering = false;
        cvf::Vec3d faceCenter = cvf::GeometryTools::computeFaceCenter(hexCorners[faceVertexIndices[0]], hexCorners[faceVertexIndices[1]], hexCorners[faceVertexIndices[2]], hexCorners[faceVertexIndices[3]]);

        for ( int i = 0; i < 4; ++i )
        {
            int next = i < 3 ? i+1 : 0;

            int intsStatus = cvf::GeometryTools::intersectLineSegmentTriangle(p1, p2,
                                                                              hexCorners[faceVertexIndices[i]],
                                                                              hexCorners[faceVertexIndices[next]],
                                                                              faceCenter,
                                                                              &intersection,
                                                                              &isEntering);
            if ( intsStatus == 1 )
            {
                uniqueIntersections.insert(HexIntersectionInfo(intersection,
                                                               isEntering,
                                                               static_cast<cvf::StructGridInterface::FaceType>(face), hexIndex));
            }
        }
    }

    int intersectionCount = 0;
    for ( const auto& intersection: uniqueIntersections)
    {
        intersections->push_back(intersection);
        ++intersectionCount;
    }

    return intersectionCount;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigHexIntersectionTools::isPointInCell(const cvf::Vec3d point, 
                                            const cvf::Vec3d hexCorners[8])
{
    cvf::Ray ray;
    ray.setOrigin(point);
    size_t intersections = 0;

    for ( int face = 0; face < 6; ++face )
    {
        cvf::ubyte faceVertexIndices[4];
        cvf::StructGridInterface::cellFaceVertexIndices(static_cast<cvf::StructGridInterface::FaceType>(face), faceVertexIndices);
        cvf::Vec3d faceCenter = cvf::GeometryTools::computeFaceCenter(hexCorners[faceVertexIndices[0]], hexCorners[faceVertexIndices[1]], hexCorners[faceVertexIndices[2]], hexCorners[faceVertexIndices[3]]);

        for ( int i = 0; i < 4; ++i )
        {
            int next = i < 3 ? i + 1 : 0;
            if ( ray.triangleIntersect(hexCorners[faceVertexIndices[i]], hexCorners[faceVertexIndices[next]], faceCenter) )
            {
                ++intersections;
            }
        }
    }
    return intersections % 2 == 1;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigHexIntersectionTools::planeHexCellIntersection(cvf::Vec3d* hexCorners, cvf::Plane fracturePlane, std::list<std::pair<cvf::Vec3d, cvf::Vec3d > >& intersectionLineSegments)
{
    bool isCellIntersected = false;
    for (int face = 0; face < 6; ++face)
    {
        cvf::ubyte faceVertexIndices[4];
        cvf::StructGridInterface::cellFaceVertexIndices(static_cast<cvf::StructGridInterface::FaceType>(face), faceVertexIndices);

        cvf::Vec3d faceCenter = cvf::GeometryTools::computeFaceCenter(hexCorners[faceVertexIndices[0]], hexCorners[faceVertexIndices[1]], hexCorners[faceVertexIndices[2]], hexCorners[faceVertexIndices[3]]);

        for (int i = 0; i < 4; i++)
        {
            int next = i < 3 ? i + 1 : 0;
            caf::HexGridIntersectionTools::ClipVx triangleIntersectionPoint1;
            caf::HexGridIntersectionTools::ClipVx triangleIntersectionPoint2;

            bool isMostVxesOnPositiveSideOfP1 = false;

            bool isIntersectingPlane = caf::HexGridIntersectionTools::planeTriangleIntersection(fracturePlane,
                hexCorners[faceVertexIndices[i]], 0,
                hexCorners[faceVertexIndices[next]], 1,
                faceCenter, 2,
                &triangleIntersectionPoint1, &triangleIntersectionPoint2, &isMostVxesOnPositiveSideOfP1);

            if (isIntersectingPlane)
            {
                isCellIntersected = true;
                intersectionLineSegments.push_back({ triangleIntersectionPoint1.vx, triangleIntersectionPoint2.vx });
            }
        }
    }    return isCellIntersected;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigHexIntersectionTools::planeHexIntersectionPolygons(std::array<cvf::Vec3d, 8> hexCorners,
                                                           cvf::Mat4d transformMatrixForPlane,
                                                           std::vector<std::vector<cvf::Vec3d> >& polygons)
{
    bool isCellIntersected = false;

    cvf::Plane fracturePlane;
    fracturePlane.setFromPointAndNormal(transformMatrixForPlane.translation(),
                                        static_cast<cvf::Vec3d>(transformMatrixForPlane.col(2)));

    //Find line-segments where cell and fracture plane intersects
    std::list<std::pair<cvf::Vec3d, cvf::Vec3d > > intersectionLineSegments;

    isCellIntersected = planeHexCellIntersection(hexCorners.data(), fracturePlane, intersectionLineSegments);

    RigCellGeometryTools::createPolygonFromLineSegments(intersectionLineSegments, polygons);

    return isCellIntersected;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool operator<(const HexIntersectionInfo& hi1, const HexIntersectionInfo& hi2)
{
    const double tolerance = 1e-6;

    if ( hi1.m_hexIndex != hi2.m_hexIndex ) return hi1.m_hexIndex < hi2.m_hexIndex;
    if ( hi1.m_face != hi2.m_face ) return hi1.m_face < hi2.m_face;
    if ( hi1.m_isIntersectionEntering != hi2.m_isIntersectionEntering ) return hi1.m_isIntersectionEntering < hi2.m_isIntersectionEntering;
    if ( hi1.m_face != hi2.m_face ) return hi1.m_face < hi2.m_face;

    if ( cvf::Math::abs(hi2.m_intersectionPoint.x() - hi1.m_intersectionPoint.x()) > tolerance ) return hi1.m_intersectionPoint.x() < hi2.m_intersectionPoint.x();
    if ( cvf::Math::abs(hi2.m_intersectionPoint.y() - hi1.m_intersectionPoint.y()) > tolerance ) return hi1.m_intersectionPoint.y() < hi2.m_intersectionPoint.y();
    if ( cvf::Math::abs(hi2.m_intersectionPoint.z() - hi1.m_intersectionPoint.z()) > tolerance ) return hi1.m_intersectionPoint.z() < hi2.m_intersectionPoint.z();

    return false;
}
