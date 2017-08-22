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

#include "RigTesselatorTools.h"

#include "cvfMath.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigEllipsisTesselator::RigEllipsisTesselator(size_t numSlices)
{
    computeCirclePoints(numSlices);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigEllipsisTesselator::tesselateEllipsis(float a, float b, std::vector<cvf::uint>* triangleIndices, std::vector<cvf::Vec3f>* nodeCoords)
{
    // See http://mathworld.wolfram.com/Ellipse.html

    for (auto v : m_circlePoints)
    {
        v.x() = v.x() * a;
        v.y() = v.y() * b;

        nodeCoords->push_back(v);
    }

    *triangleIndices = m_circleConnectivities;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigEllipsisTesselator::computeCirclePoints(size_t numSlices)
{
    // Based on GeometryUtils::generatePointsOnCircle 

    double da = 2 * cvf::PI_D / numSlices;
     
    cvf::Vec3f point = cvf::Vec3f::ZERO;

    // Center of circle
    m_circlePoints.push_back(point);

    for (size_t i = 0; i < numSlices; i++) 
    {
        // Precompute this one (A = i*da;)
        double sinA = cvf::Math::sin(i*da);
        double cosA = cvf::Math::cos(i*da);

        point.x() = static_cast<float>(-sinA);
        point.y() = static_cast<float>( cosA);

        m_circlePoints.push_back(point);
    }

    for (cvf::uint i = 0; i < static_cast<cvf::uint>(numSlices); i++)
    {
        // Center
        m_circleConnectivities.push_back(0);

        m_circleConnectivities.push_back(i + 1);

        if (i == (numSlices - 1))
        {
            // Connect the last slice to the first slice
            m_circleConnectivities.push_back(1);
        }
        else
        {
            m_circleConnectivities.push_back(i + 2);
        }
    }
}
