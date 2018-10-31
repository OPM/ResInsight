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

#include <array>
#include <list>
#include <vector>



class RigCellGeometryTools
{
public:

    static void createPolygonFromLineSegments(std::list<std::pair<cvf::Vec3d, cvf::Vec3d>> &intersectionLineSegments, std::vector<std::vector<cvf::Vec3d>> &polygons);

    static void findCellLocalXYZ(const std::array<cvf::Vec3d, 8>& hexCorners, cvf::Vec3d &localXdirection, cvf::Vec3d &localYdirection, cvf::Vec3d &localZdirection);

    static double polygonLengthInLocalXdirWeightedByArea(const std::vector<cvf::Vec3d>& polygon2d);

    static std::vector<std::vector<cvf::Vec3d>> intersectPolygons(const std::vector<cvf::Vec3d>& polygon1,
                                                                  const std::vector<cvf::Vec3d>& polygon2);
    
    static std::vector<std::vector<cvf::Vec3d>> subtractPolygons(const std::vector<cvf::Vec3d>&              sourcePolygon,
                                                                 const std::vector<std::vector<cvf::Vec3d>>& polygonToSubtract);

    enum ZInterpolationType { INTERPOLATE_LINE_Z, USE_HUGEVAL, USE_ZERO};
    static std::vector<std::vector<cvf::Vec3d> >  clipPolylineByPolygon(const std::vector<cvf::Vec3d>& polyLine, 
                                                                        const std::vector<cvf::Vec3d>& polygon, 
                                                                        ZInterpolationType interpolType = USE_ZERO);

    static std::pair<cvf::Vec3d, cvf::Vec3d> getLineThroughBoundingBox(const cvf::Vec3d& lineDirection, const cvf::BoundingBox& polygonBBox, const cvf::Vec3d& pointOnLine);

    static double getLengthOfPolygonAlongLine(const std::pair<cvf::Vec3d, cvf::Vec3d>& line, const std::vector<cvf::Vec3d>& polygon);

    static std::vector<cvf::Vec3d> unionOfPolygons(const std::vector<std::vector<cvf::Vec3d>>& polygons);

private:
    static std::vector<cvf::Vec3d> ajustPolygonToAvoidIntersectionsAtVertex(const std::vector<cvf::Vec3d>& polyLine, 
                                                                            const std::vector<cvf::Vec3d>& polygon);

};
