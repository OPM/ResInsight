/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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

#include "RigFractureCell.h"

#include "RiaLogging.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFractureCell::RigFractureCell(std::vector<cvf::Vec3d> polygon, size_t i, size_t j)
    : m_polygon(polygon)
    , m_i(i)
    , m_j(j)
    , m_concutivityValue(0.0)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::Vec3d>& RigFractureCell::getPolygon() const
{
    return m_polygon;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigFractureCell::getConductivtyValue() const
{
    return m_concutivityValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigFractureCell::getI() const
{
    return m_i;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigFractureCell::getJ() const
{
    return m_j;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFractureCell::hasNonZeroConductivity() const
{
    return m_concutivityValue > 1e-7;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFractureCell::setConductivityValue(double cond)
{
    m_concutivityValue = cond;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigFractureCell::cellSizeX() const
{
    // The polygon corners are always stored in the same order
    if (m_polygon.size() > 1) return (m_polygon[1] - m_polygon[0]).length();
    return cvf::UNDEFINED_DOUBLE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigFractureCell::cellSizeZ() const
{
    if (m_polygon.size() > 2) return (m_polygon[2] - m_polygon[1]).length();
    return cvf::UNDEFINED_DOUBLE;
}
