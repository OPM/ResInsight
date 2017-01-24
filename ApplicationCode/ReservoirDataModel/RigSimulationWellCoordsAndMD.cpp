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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigSimulationWellCoordsAndMD::RigSimulationWellCoordsAndMD(const std::vector<cvf::Vec3d>& wellPathPoints)
{
    m_wellPathPoints = wellPathPoints;

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
