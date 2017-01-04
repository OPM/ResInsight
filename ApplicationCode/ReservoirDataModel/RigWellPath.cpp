/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RigWellPath.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigWellPath::RigWellPath()
    : m_hasDatumElevation(false),
    m_datumElevation(0.0)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigWellPath::setDatumElevation(double value)
{
    m_hasDatumElevation = true;
    m_datumElevation = value;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigWellPath::hasDatumElevation() const
{
    return m_hasDatumElevation;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigWellPath::datumElevation() const
{
    return m_datumElevation;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigWellPath::interpolatedPointAlongWellPath(double measuredDepth)
{
    cvf::Vec3d wellPathPoint = cvf::Vec3d::ZERO;
    if (measuredDepth < 0) return wellPathPoint;

    int i = 0;
    while (i < m_measuredDepths.size() && m_measuredDepths.at(i) < measuredDepth )
    {
        i++;
    }

    if (m_measuredDepths.size() > i)
    {
        if (i == 0)
        {
            //For measuredDepth=0 use startpoint
            wellPathPoint = m_wellPathPoints.at(0);
        }
        else
        {
            //Do interpolation
            double stepsize = (measuredDepth - m_measuredDepths.at(i-1)) / 
                                        (m_measuredDepths.at(i) - m_measuredDepths.at(i - 1));
            wellPathPoint = m_wellPathPoints.at(i - 1) + stepsize * (m_wellPathPoints.at(i) - m_wellPathPoints.at(i-1));
        }
    }
    else
    {
        //Use endpoint if measuredDepth same or higher than last point
        wellPathPoint = m_wellPathPoints.at(i-1);
    }


    return wellPathPoint;
}

