/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

//--------------------------------------------------------------------------------------------------
/// Lets you estimate MD values from an existing md/tvd relationship and a new set of TVD-values
/// Requires the points to be ordered from the start/top of the well path to the end/bottom.
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellPathGeometryTools::interpolateMdFromTvd(const std::vector<double>& originalMdValues, const std::vector<double>& originalTvdValues, const std::vector<double>& tvdValuesToInterpolateFrom)
{
    CVF_ASSERT(!originalMdValues.empty());
    if (originalMdValues.size() < 2u)
    {
        return {originalMdValues};
    }

    std::vector<double> interpolatedMdValues;
    interpolatedMdValues.reserve(tvdValuesToInterpolateFrom.size());

    std::vector<double>::const_iterator last_it = originalTvdValues.begin();
    for (std::vector<double>::const_iterator it = tvdValuesToInterpolateFrom.begin(); it != tvdValuesToInterpolateFrom.end(); ++it)
    {
        double tvdValue = *it;
        double sign = 0.0;
        if (it != tvdValuesToInterpolateFrom.begin())
        {
            sign = *it - *(it - 1);
        }
        else
        {
            sign = *(it + 1) - *it;
        }
        if (std::fabs(sign) < 1.0e-8)
        {
            continue;
        }
            
        sign /= std::fabs(sign);

        auto current_it = last_it;
        // Is incrementing current_it taking us closer to the TVD value we want?
        while (current_it != originalTvdValues.end())
        {
            if (*current_it * sign >= tvdValue * sign)
            {
                break;
            }

            auto next_it = current_it + 1;
            if (next_it != originalTvdValues.end())
            {                
                double originalDataSign = (*next_it - *current_it);
                originalDataSign /= std::fabs(originalDataSign);
                if (originalDataSign * sign < 0.0)
                    break;
            }
            current_it = next_it;
        }

        int valueIndex = static_cast<int>(current_it - originalTvdValues.begin());
        double mdValue = linearInterpolation(originalTvdValues, originalMdValues, valueIndex, tvdValue);
        interpolatedMdValues.push_back(mdValue);
        last_it = current_it;
    }
    return interpolatedMdValues;
}

std::vector<cvf::Vec3d> RigWellPathGeometryTools::interpolateUndefinedNormals(const cvf::Vec3d&              planeNormal,
                                                                              const std::vector<cvf::Vec3d>& normals,
                                                                              const std::vector<cvf::Vec3d>& vertices)
{
    std::vector<cvf::Vec3d> interpolated(normals);
    cvf::Vec3d              lastNormalNonInterpolated(0, 0, 0);
    cvf::Vec3d              lastNormalAny(0, 0, 0);
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

            if (lastNormalNonInterpolated.length() > 0.0 && nextNormal.length() > 0.0)
            {
                // Both last and next are acceptable, interpolate!
                currentNormal = (distanceToNext * lastNormalNonInterpolated + distanceFromLast * nextNormal).getNormalized();
            }
            else if (lastNormalNonInterpolated.length() > 0.0)
            {
                currentNormal = lastNormalNonInterpolated;
            }
            else if (nextNormal.length() > 0.0)
            {
                currentNormal = nextNormal;
            }
        }
        if (i > 0 && currentNormal * lastNormalAny < -std::numeric_limits<double>::epsilon())
        {
            currentNormal *= -1.0;
        }
        if (!currentInterpolated)
        {
            lastNormalNonInterpolated = currentNormal;
            distanceFromLast = 0.0; // Reset distance
        }
        lastNormalAny = currentNormal;
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigWellPathGeometryTools::linearInterpolation(const std::vector<double>& xValues, const std::vector<double>& yValues, int valueIndex, double x)
{
    int N = (int) xValues.size() - 1;
    int i = cvf::Math::clamp(valueIndex, 0, N);
    
    std::vector<double> interpolatedValues;
    // Backwards
    if (i > 0)
    {
        double x1 = xValues[i - 1];
        double x2 = xValues[i];
        double y1 = yValues[i - 1];
        double y2 = yValues[i];
        interpolatedValues.push_back(linearInterpolation(x1, x2, y1, y2, x));
    }
    if (i < N)
    {
        double x1 = xValues[i];
        double x2 = xValues[i + 1];
        double y1 = yValues[i];
        double y2 = yValues[i + 1];
        interpolatedValues.push_back(linearInterpolation(x1, x2, y1, y2, x));
    }

    double sum = 0.0;
    for (double value : interpolatedValues)
    {
        sum += value;
    }
    return sum / (double) interpolatedValues.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigWellPathGeometryTools::linearInterpolation(double x1, double x2, double y1, double y2, double x)
{
    double M = (y2 - y1) / (x2 - x1);
    return M * (x - x1) + y1;
}
