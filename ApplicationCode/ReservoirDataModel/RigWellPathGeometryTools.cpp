/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "RigWellPathGeometryTools.h"

#include "RigWellPath.h"

#include "cvfMatrix3.h"
#include "cvfMath.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RigWellPathGeometryTools::calculateLineSegmentNormals(const RigWellPath*             wellPathGeometry,
                                                                              double                         angle,
                                                                              const std::vector<cvf::Vec3d>& vertices,
                                                                              VertexOrganization             organization)
{
    std::vector<cvf::Vec3d> pointNormals;

    if (!wellPathGeometry) return pointNormals;
    if (vertices.empty()) return pointNormals;

    const cvf::Vec3d globalDirection =
        (wellPathGeometry->m_wellPathPoints.back() - wellPathGeometry->m_wellPathPoints.front()).getNormalized();

    const cvf::Vec3d up(0, 0, 1);

    size_t intervalSize;
    if (organization == LINE_SEGMENTS)
    {
        pointNormals.reserve(vertices.size() / 2);
        intervalSize = 2;
    }
    else // organization == POLYLINE
    {
        pointNormals.reserve(vertices.size());
        intervalSize = 1;
    }

    cvf::Vec3d normal;

    for (size_t i = 0; i < vertices.size() - 1; i += intervalSize)
    {
        cvf::Vec3d p1 = vertices[i];
        cvf::Vec3d p2 = vertices[i + 1];

        cvf::Vec3d vecAlongPath = (p2 - p1).getNormalized();

        double dotProduct = up * vecAlongPath;

        cvf::Vec3d Ex;

        if (cvf::Math::abs(dotProduct) > 0.7071)
        {
            Ex = globalDirection;
        }
        else
        {
            Ex = vecAlongPath;
        }

        cvf::Vec3d Ey = (up ^ Ex).getNormalized();

        cvf::Mat3d rotation;
        normal = Ey.getTransformedVector(rotation.fromRotation(Ex, angle));

        pointNormals.push_back(normal);
    }

    if (organization == POLYLINE)
    {
        pointNormals.push_back(normal);
    }

    return pointNormals;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigWellPathGeometryTools::calculatePairsOfClosestSamplingPointsAlongWellPath(const RigWellPath* wellPathGeometry, std::vector<cvf::Vec3d>* closestWellPathPoints, std::vector<cvf::Vec3d>& points)
{
    CVF_ASSERT(closestWellPathPoints != nullptr);

    for (const cvf::Vec3d point : points)
    {
        cvf::Vec3d p1 = cvf::Vec3d::UNDEFINED;
        cvf::Vec3d p2 = cvf::Vec3d::UNDEFINED;
        wellPathGeometry->twoClosestPoints(point, &p1, &p2);
        if (p1.isUndefined() || p2.isUndefined()) continue;

        closestWellPathPoints->push_back(p1);
        closestWellPathPoints->push_back(p2);
    }
}
