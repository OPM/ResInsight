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

#include "cvfMath.h"
#include "cvfMatrix3.h"

#include <algorithm>
#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RigWellPathGeometryTools::calculateLineSegmentNormals(const std::vector<cvf::Vec3d>& vertices,
                                                                              double                         planeAngle)
{
    std::vector<cvf::Vec3d> pointNormals;

    if (vertices.empty()) return pointNormals;

    pointNormals.reserve(vertices.size());

    const cvf::Vec3d up(0, 0, 1);
    const cvf::Vec3d rotatedUp = up.getTransformedVector(cvf::Mat3d::fromRotation(cvf::Vec3d(0.0, 1.0, 0.0), planeAngle));


    const cvf::Vec3d dominantDirection = estimateDominantDirectionInXYPlane(vertices);

    const cvf::Vec3d projectionPlaneNormal = (up ^ dominantDirection).getNormalized();
    CVF_ASSERT(projectionPlaneNormal * dominantDirection <= std::numeric_limits<double>::epsilon());

    double sumDotWithRotatedUp = 0.0;
    for (size_t i = 0; i < vertices.size() - 1; ++i)
    {
        cvf::Vec3d p1 = vertices[i];
        cvf::Vec3d p2 = vertices[i + 1];

        cvf::Vec3d tangent = (p2 - p1).getNormalized();
        cvf::Vec3d normal(0, 0, 0);
        if (cvf::Math::abs(tangent * projectionPlaneNormal) < 0.7071)
        {
            cvf::Vec3d projectedTangent = (tangent - (tangent * projectionPlaneNormal) * projectionPlaneNormal).getNormalized();
            normal                      = (projectedTangent ^ projectionPlaneNormal).getNormalized();
            normal                      = normal.getTransformedVector(cvf::Mat3d::fromRotation(tangent, planeAngle));
        }
        pointNormals.push_back(normal);
        sumDotWithRotatedUp += normal * rotatedUp;
    }
    
    pointNormals.push_back(pointNormals.back());

    if (sumDotWithRotatedUp < 0.0)
    {
        for (cvf::Vec3d& normal : pointNormals)
        {
            normal *= -1.0;            
        }
    }


    return interpolateUndefinedNormals(up, pointNormals, vertices);
}

std::vector<cvf::Vec3d> RigWellPathGeometryTools::interpolateUndefinedNormals(const cvf::Vec3d&              planeNormal,
                                                                              const std::vector<cvf::Vec3d>& normals,
                                                                              const std::vector<cvf::Vec3d>& vertices)
{
    std::vector<cvf::Vec3d> interpolated(normals);
    cvf::Vec3d              lastNormal(0, 0, 0);
    cvf::Vec3d              lastNormalIncludingInterpolated(0, 0, 0);
    double distanceFromLast = 0.0;

    for (size_t i = 0; i < normals.size(); ++i)
    {
        cvf::Vec3d currentNormal       = normals[i];
        bool       currentInterpolated = false;
        if (i > 0)
        {
            distanceFromLast += (vertices[i] - vertices[i - 1]).length();
        }

        if (currentNormal.length() == 0.0) // Undefined. Need to estimate from neighbors.
        {
            currentInterpolated = true;
            currentNormal = planeNormal; // By default use the plane normal

            cvf::Vec3d nextNormal(0, 0, 0);
            double     distanceToNext = 0.0;
            for (size_t j = i + 1; j < normals.size() && nextNormal.length() == 0.0; ++j)
            {
                nextNormal = normals[j];
                distanceToNext += (vertices[j] - vertices[j - 1]).length();
            }

            if (lastNormal.length() > 0.0 && nextNormal.length() > 0.0)
            {
                // Both last and next are acceptable, interpolate!
                currentNormal = (distanceToNext * lastNormal + distanceFromLast * nextNormal).getNormalized();
            }
            else if (lastNormal.length() > 0.0)
            {
                currentNormal = lastNormal;
            }
            else if (nextNormal.length() > 0.0)
            {
                currentNormal = nextNormal;
            }
        }
        if (i > 0 && currentNormal * lastNormalIncludingInterpolated < -std::numeric_limits<double>::epsilon())
        {
            currentNormal *= -1.0;
        }
        if (!currentInterpolated)
        {
            lastNormal       = currentNormal;
            distanceFromLast = 0.0; // Reset distance
        }
        lastNormalIncludingInterpolated = currentNormal;
        interpolated[i] = currentNormal;
    }
    return interpolated;
}

cvf::Vec3d RigWellPathGeometryTools::estimateDominantDirectionInXYPlane(const std::vector<cvf::Vec3d>& vertices)
{
    cvf::Vec3d directionSum(0, 0, 0);
    for (size_t i = 1; i < vertices.size(); ++i)
    {
        cvf::Vec3d vec = vertices[i] - vertices[i - 1];
        vec.z()        = 0.0;
        if (directionSum.length() > 0.0 && (directionSum * vec) < 0.0)
        {
            vec *= -1;
        }
        directionSum += vec;
    }
    
    if (directionSum.length() < 1.0e-8)
    {
        directionSum = cvf::Vec3d(0, -1, 0);
    }

    return directionSum.getNormalized();
}
