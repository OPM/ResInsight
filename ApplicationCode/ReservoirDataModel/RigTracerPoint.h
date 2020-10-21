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

#pragma once

#include "cvfVector3.h"

//==================================================================================================
///  Class representing one single point in a streamline tracer. The point has a position and
///    and a direction vector. The absolute value of the direction vector is precalculated to save
///    some calculation time later on.
//==================================================================================================
class RigTracerPoint
{
public:
    RigTracerPoint( cvf::Vec3d position, cvf::Vec3d direction );
    ~RigTracerPoint();

    const cvf::Vec3d& position() const;
    const cvf::Vec3d& direction() const;
    double            absValue() const;

private:
    cvf::Vec3d m_position;
    cvf::Vec3d m_direction;
    double     m_absValue;
};
