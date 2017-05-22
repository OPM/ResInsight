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

#include "cvfBase.h"
#include "cvfVector3.h"

#include "cvfBoundingBox.h"
#include "cvfPlane.h"

#include <list>
#include <vector>



class RigCellGeometryTools
{
public:
    RigCellGeometryTools();
    ~RigCellGeometryTools(); 

    static bool planeHexCellIntersection(cvf::Vec3d * hexCorners, cvf::Plane fracturePlane, std::list<std::pair<cvf::Vec3d, cvf::Vec3d > > & intersectionLineSegments);

    static void createPolygonFromLineSegments(std::list<std::pair<cvf::Vec3d, cvf::Vec3d>> &intersectionLineSegments, std::vector<std::vector<cvf::Vec3d>> &polygons);

    static void findCellLocalXYZ(cvf::Vec3d * hexCorners, cvf::Vec3d &localXdirection, cvf::Vec3d &localYdirection, cvf::Vec3d &localZdirection);
    
    static double polygonAreaWeightedLength(cvf::Vec3d directionOfLength, std::vector<cvf::Vec3d> polygon2d);
   
    static std::vector<std::vector<cvf::Vec3d> >  intersectPolygons(std::vector<cvf::Vec3d> polygon1, std::vector<cvf::Vec3d> polygon2);

    enum ZInterpolationType { INTERPOLATE_LINE_Z, USE_HUGEVAL, USE_ZERO};
    static std::vector<std::vector<cvf::Vec3d> >  clipPolylineByPolygon(std::vector<cvf::Vec3d> polyLine, std::vector<cvf::Vec3d> polygon, ZInterpolationType interpolType = USE_ZERO);

    static std::pair<cvf::Vec3d, cvf::Vec3d> getLineThroughBoundingBox(cvf::Vec3d lineDirection, cvf::BoundingBox polygonBBox, cvf::Vec3d pointOnLine);

    static double getLengthOfPolygonAlongLine(std::pair<cvf::Vec3d, cvf::Vec3d> line, std::vector<cvf::Vec3d> polygon);
};
