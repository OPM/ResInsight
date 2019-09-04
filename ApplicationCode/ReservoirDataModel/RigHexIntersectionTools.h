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
#pragma once


#include "cvfVector3.h"
#include "cvfStructGrid.h"
#include "cvfPlane.h"

#include <array>

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

bool operator<( const HexIntersectionInfo& hi1, const HexIntersectionInfo& hi2);

//--------------------------------------------------------------------------------------------------
/// Specialized Line - Hex intersection
//--------------------------------------------------------------------------------------------------
struct RigHexIntersectionTools
{
    static int lineHexCellIntersection(const cvf::Vec3d p1, 
                                       const cvf::Vec3d p2, 
                                       const cvf::Vec3d hexCorners[8], 
                                       const size_t hexIndex,
                                       std::vector<HexIntersectionInfo>* intersections);

     static bool lineIntersectsHexCell(const cvf::Vec3d p1, 
                                       const cvf::Vec3d p2, 
                                       const cvf::Vec3d hexCorners[8]);

    static bool isPointInCell(const cvf::Vec3d point, const cvf::Vec3d hexCorners[8]);

    static bool planeHexCellIntersection(cvf::Vec3d* hexCorners,
                                         const cvf::Plane& fracturePlane,
                                         std::list<std::pair<cvf::Vec3d, cvf::Vec3d > >& intersectionLineSegments);

    static bool planeHexIntersectionPolygons(std::array<cvf::Vec3d, 8> hexCorners,
                                             cvf::Mat4d transformMatrixForPlane,
                                             std::vector<std::vector<cvf::Vec3d> > & polygons);
};


