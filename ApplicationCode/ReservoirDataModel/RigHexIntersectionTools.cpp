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

#include "cvfBoundingBox.h"
#include "cvfGeometryTools.h"
#include "cvfRay.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RigHexIntersectionTools::lineHexCellIntersection(const cvf::Vec3d p1, const cvf::Vec3d p2, const cvf::Vec3d hexCorners[8], const size_t hexIndex, std::vector<HexIntersectionInfo>* intersections)
{
    CVF_ASSERT(intersections != NULL);

    int intersectionCount = 0;

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
                intersectionCount++;
                intersections->push_back(HexIntersectionInfo(intersection,
                                                             isEntering,
                                                             static_cast<cvf::StructGridInterface::FaceType>(face), hexIndex));
            }
        }
    }

    return intersectionCount;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigHexIntersectionTools::isPointInCell(const cvf::Vec3d point, const cvf::Vec3d hexCorners[8])
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
