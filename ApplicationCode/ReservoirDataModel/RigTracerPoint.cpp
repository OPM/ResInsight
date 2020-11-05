/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 - Equinor ASA
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

#include "RigTracerPoint.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigTracerPoint::RigTracerPoint( cvf::Vec3d position, cvf::Vec3d direction )
    : m_position( position )
    , m_direction( direction )
{
    m_absValue = m_direction.length();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigTracerPoint::~RigTracerPoint()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::Vec3d& RigTracerPoint::position() const
{
    return m_position;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::Vec3d& RigTracerPoint::direction() const
{
    return m_direction;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigTracerPoint::absValue() const
{
    return m_absValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigTracerPoint::reverse()
{
    m_direction *= -1.0;
}
