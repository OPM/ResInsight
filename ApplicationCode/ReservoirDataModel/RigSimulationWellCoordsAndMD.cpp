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

#include "RigSimulationWellCoordsAndMD.h"

#include "cvfGeometryTools.h"
#include "cvfMath.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigSimulationWellCoordsAndMD::RigSimulationWellCoordsAndMD(const std::vector<cvf::Vec3d>& wellPathPoints)
: m_wellPathPoints(wellPathPoints)
{
    computeMeasuredDepths();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::Vec3d>& RigSimulationWellCoordsAndMD::wellPathPoints() const
{
    return m_wellPathPoints;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigSimulationWellCoordsAndMD::measuredDepths() const
{
    return m_measuredDepths;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigSimulationWellCoordsAndMD::interpolatedPointAlongWellPath(double measuredDepth) const
{
    cvf::Vec3d wellPathPoint = cvf::Vec3d::ZERO;

    size_t i = 0;
    while (i < m_measuredDepths.size() && m_measuredDepths.at(i) < measuredDepth)
    {
        i++;
    }

    if (m_measuredDepths.size() > i)
    {
        if (i == 0)
        {
            //For measuredDepth same or lower than first point, use this first point
            wellPathPoint = m_wellPathPoints.at(0);
        }
        else
        {
            //Do interpolation
            double stepsize = (measuredDepth - m_measuredDepths.at(i - 1)) /
                (m_measuredDepths.at(i) - m_measuredDepths.at(i - 1));
            wellPathPoint = m_wellPathPoints.at(i - 1) + stepsize * (m_wellPathPoints.at(i) - m_wellPathPoints.at(i - 1));
        }
    }
    else
    {
        //Use endpoint if measuredDepth same or higher than last point
        wellPathPoint = m_wellPathPoints.at(i - 1);
    }

    return wellPathPoint;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigSimulationWellCoordsAndMD::locationAlongWellCoords(const cvf::Vec3d& position) const
{
    double location = 0.0;
    
    size_t closestIndex = findClosestIndex(position);

    if (closestIndex != cvf::UNDEFINED_DOUBLE)
    {
        cvf::Vec3d p1 = m_wellPathPoints[closestIndex - 1];
        cvf::Vec3d p2 = m_wellPathPoints[closestIndex - 0];

        double intersection = 0.0;
        cvf::GeometryTools::projectPointOnLine(p1, p2, position, &intersection);

        location = m_measuredDepths[closestIndex - 1];
        location += intersection * (p1-p2).length();
    }

    return location;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigSimulationWellCoordsAndMD::findClosestIndex(const cvf::Vec3d& position) const
{
    size_t closestIndex = cvf::UNDEFINED_SIZE_T;
    double closestDistance = cvf::UNDEFINED_DOUBLE;

    for (size_t i = 1; i < m_wellPathPoints.size(); i++)
    {
        cvf::Vec3d p1 = m_wellPathPoints[i - 1];
        cvf::Vec3d p2 = m_wellPathPoints[i - 0];

        double candidateDistance = cvf::GeometryTools::linePointSquareDist(p1, p2, position);
        if (candidateDistance < closestDistance)
        {
            closestDistance = candidateDistance;
            closestIndex = i;
        }
    }
    return closestIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigSimulationWellCoordsAndMD::simWellAzimuthAngle(const cvf::Vec3d& position) const
{
    size_t closestIndex = findClosestIndex(position);

     //For vertical well (x-component of direction = 0) returned angle will be 90. 
    double azimuthAngle = 90.0;

    if (closestIndex != cvf::UNDEFINED_DOUBLE)
    {
        cvf::Vec3d p1;
        cvf::Vec3d p2;

        if (closestIndex > 0)
        {
            p1 = m_wellPathPoints[closestIndex - 1];
            p2 = m_wellPathPoints[closestIndex - 0];
        }
        else
        {
            p1 = m_wellPathPoints[closestIndex + 1];
            p2 = m_wellPathPoints[closestIndex + 0];
        }

        cvf::Vec3d direction = p2 - p1;


        if (abs(direction.y()) > 1e-5) 
         {
             double atanValue = direction.x() / direction.y();
             azimuthAngle = atan(atanValue);
             azimuthAngle = cvf::Math::toDegrees(azimuthAngle);
         }
     }

    return azimuthAngle;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigSimulationWellCoordsAndMD::simWellDipAngle(const cvf::Vec3d& position) const
{
    size_t closestIndex = findClosestIndex(position);

    double dipAngle = 0.0;

    if (closestIndex != cvf::UNDEFINED_DOUBLE)
    {
        cvf::Vec3d p1;
        cvf::Vec3d p2;

        if (closestIndex > 0)
        {
            p1 = m_wellPathPoints[closestIndex - 1];
            p2 = m_wellPathPoints[closestIndex - 0];
        }
        else
        {
            p1 = m_wellPathPoints[closestIndex + 1];
            p2 = m_wellPathPoints[closestIndex + 0];
        }

        cvf::Vec3d direction = p1 - p2;

        double horizonal = sqrt(pow(direction.x(), 2) + pow(direction.y(), 2));
        double vertical = direction.z();

        if (abs(vertical) > 1e-5)
        {
            double atanValue =  vertical / horizonal;
            dipAngle = atan(atanValue);
            dipAngle = cvf::Math::toDegrees(dipAngle);
        }
    }

    return dipAngle;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigSimulationWellCoordsAndMD::computeMeasuredDepths()
{
    cvf::Vec3d prev = cvf::Vec3d::UNDEFINED;

    double accumulatedMD = 0;

    for (const auto& point : m_wellPathPoints)
    {
        if (!prev.isUndefined())
        {
            accumulatedMD += point.pointDistance(prev);
        }

        m_measuredDepths.push_back(accumulatedMD);

        prev = point;
    }
}
