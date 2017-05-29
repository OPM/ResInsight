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
#include "cvfRay.h"

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
                                                                                  hexCorners[faceVertexIndices[i]], 
                                                                                  hexCorners[faceVertexIndices[next]], 
                                                                                  faceCenter,
                                                                                  &intersection,
                                                                                  &isEntering);
                if (intsStatus == 1)
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

    static bool isPointInCell(const cvf::Vec3d point, const cvf::Vec3d hexCorners[8])
    {
        cvf::Ray ray;
        ray.setOrigin(point);
        size_t intersections = 0;

        for (int face = 0; face < 6; ++face)
        {
            cvf::ubyte faceVertexIndices[4];
            cvf::StructGridInterface::cellFaceVertexIndices(static_cast<cvf::StructGridInterface::FaceType>(face), faceVertexIndices);
            cvf::Vec3d faceCenter = cvf::GeometryTools::computeFaceCenter(hexCorners[faceVertexIndices[0]], hexCorners[faceVertexIndices[1]], hexCorners[faceVertexIndices[2]], hexCorners[faceVertexIndices[3]]);

            for (int i = 0; i < 4; ++i)
            {
                int next = i < 3 ? i + 1 : 0;
                if (ray.triangleIntersect(hexCorners[faceVertexIndices[i]], hexCorners[faceVertexIndices[next]], faceCenter))
                {
                    ++intersections;
                }
            }
        }
        return intersections % 2 == 1;
    }

    static bool isEqualDepth(double d1, double d2) 
    { 
        double depthDiff = d1 - d2;

        const double tolerance = 0.1;// Meters To handle inaccuracies across faults

        return (fabs(depthDiff) < tolerance); // Equal depth
    }
};

//==================================================================================================
///  Class used to sort the intersections along the wellpath,
///  Use as key in a map
/// Sorting according to MD first, then Cell Idx, then Leaving before entering cell
//==================================================================================================

struct RigMDCellIdxEnterLeaveKey
{
    RigMDCellIdxEnterLeaveKey(double md, size_t cellIdx, bool entering): measuredDepth(md), hexIndex(cellIdx), isEnteringCell(entering){}

    double measuredDepth;
    size_t hexIndex;
    bool   isEnteringCell; // As opposed to leaving.
    bool   isLeavingCell() const { return !isEnteringCell;}

    bool operator < (const RigMDCellIdxEnterLeaveKey& other) const 
    {
        if (RigHexIntersector::isEqualDepth(measuredDepth, other.measuredDepth))
        {
            if (hexIndex == other.hexIndex)
            {
                if (isEnteringCell == other.isEnteringCell)
                {
                    // Completely equal: intersections at cell edges or corners or edges of the face triangles
                    return false;
                }

                return !isEnteringCell; // Sort Leaving cell before (less than) entering cell
            }

            return (hexIndex < other.hexIndex);
        }

        // The depths are not equal

        return (measuredDepth < other.measuredDepth);
    }
};


//==================================================================================================
///  Class used to sort the intersections along the wellpath,
///  Use as key in a map
/// Sorting according to MD first,then Leaving before entering cell, then Cell Idx, 
//==================================================================================================

struct RigMDEnterLeaveCellIdxKey
{
    RigMDEnterLeaveCellIdxKey(double md, bool entering, size_t cellIdx ): measuredDepth(md), hexIndex(cellIdx), isEnteringCell(entering){}

    double measuredDepth;
    bool   isEnteringCell; // As opposed to leaving.
    bool   isLeavingCell() const { return !isEnteringCell;}
    size_t hexIndex;

    bool operator < (const RigMDEnterLeaveCellIdxKey& other) const
    {
        if (RigHexIntersector::isEqualDepth(measuredDepth, other.measuredDepth))
        {
            if (isEnteringCell == other.isEnteringCell)
            {
                if (hexIndex == other.hexIndex)
                {
                    // Completely equal: intersections at cell edges or corners or edges of the face triangles
                    return false;
                }

                return (hexIndex < other.hexIndex);
            }

            return isLeavingCell(); // Sort Leaving cell before (less than) entering cell
        }

        // The depths are not equal

        return (measuredDepth < other.measuredDepth);
    }

    static bool isProperCellEnterLeavePair(const RigMDEnterLeaveCellIdxKey& key1, const RigMDEnterLeaveCellIdxKey& key2 )
    {
        return ( key1.hexIndex == key2.hexIndex 
                && key1.isEnteringCell && key2.isLeavingCell()
                && !RigHexIntersector::isEqualDepth(key1.measuredDepth, key2.measuredDepth));
    }
};


