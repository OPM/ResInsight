/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#pragma once
#include "cvfBoundingBox.h"
#include "cvfGeometryTools.h"
#include "cvfStructGrid.h"

//==================================================================================================
///  Internal class for intersection point info 
//==================================================================================================

struct HexIntersectionInfo
{

public:
    HexIntersectionInfo( cvf::Vec3d                          intersectionPoint,
                         bool                                isIntersectionEntering,
                         cvf::StructGridInterface::FaceType  face,
                         size_t                              hexIndex) 
                         : m_intersectionPoint(intersectionPoint),
                           m_isIntersectionEntering(isIntersectionEntering),
                           m_face(face),
                           m_hexIndex(hexIndex) {}


    cvf::Vec3d                          m_intersectionPoint;
    bool                                m_isIntersectionEntering;
    cvf::StructGridInterface::FaceType  m_face;
    size_t                              m_hexIndex;
};

//--------------------------------------------------------------------------------------------------
/// Specialized Line - Hex intersection
//--------------------------------------------------------------------------------------------------
struct RigHexIntersector
{
    static int lineHexCellIntersection(const cvf::Vec3d p1, const cvf::Vec3d p2, const cvf::Vec3d hexCorners[8], const size_t hexIndex,
                                       std::vector<HexIntersectionInfo>* intersections)
    {
        CVF_ASSERT(intersections != NULL);

        int intersectionCount = 0;

        for (int face = 0; face < 6 ; ++face)
        {
            cvf::ubyte faceVertexIndices[4];
            cvf::StructGridInterface::cellFaceVertexIndices(static_cast<cvf::StructGridInterface::FaceType>(face), faceVertexIndices);

            cvf::Vec3d intersection;
            bool isEntering = false;
            cvf::Vec3d faceCenter = cvf::GeometryTools::computeFaceCenter(hexCorners[faceVertexIndices[0]], hexCorners[faceVertexIndices[1]], hexCorners[faceVertexIndices[2]], hexCorners[faceVertexIndices[3]]);

            for (int i = 0; i < 4; ++i)
            {
                int next = i < 3 ? i+1 : 0;

                int intsStatus = cvf::GeometryTools::intersectLineSegmentTriangle(p1, p2,
                                                                                  hexCorners[faceVertexIndices[i]], hexCorners[faceVertexIndices[next]], faceCenter,
                                                                                  &intersection,
                                                                                  &isEntering);
                if (intsStatus == 1)
                {
                    intersectionCount++;
                    intersections->push_back(HexIntersectionInfo(intersection, isEntering, static_cast<cvf::StructGridInterface::FaceType>(face), hexIndex));
                }
            }
        }

        return intersectionCount;
    }
};

//==================================================================================================
///  Class used to sort the intersections along the wellpath
//==================================================================================================

struct WellPathDepthPoint
{
    WellPathDepthPoint(double md, bool entering): measuredDepth(md), isEnteringCell(entering){}

    double measuredDepth;
    bool   isEnteringCell; // As opposed to leaving.

    bool operator < (const WellPathDepthPoint& other) const 
    {
        double depthDiff = other.measuredDepth - measuredDepth;

        const double tolerance = 1e-6;

        if (fabs(depthDiff) < tolerance) // Equal depth
        {
            if (isEnteringCell == other.isEnteringCell)
            {
                // Completely equal, probably hitting between two cells
                return false;
            }

            if(!isEnteringCell) // Leaving this cell
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        // The depths are not equal

        if (measuredDepth < other.measuredDepth)
        {
            return true;
        }
        else if (measuredDepth > other.measuredDepth)
        {
            return false;
        }

        CVF_ASSERT(false);
        return false;
    }
};


