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

#include "cvfObject.h"
#include "cvfVector3.h"

#include <vector>

class RigTracerPoint;

//==================================================================================================
/// Class representing one streamline tracer line, with position and direction given for each
///  time increment.
//==================================================================================================
class RigTracer : public cvf::Object
{
public:
    RigTracer();
    explicit RigTracer( const RigTracer& other );
    ~RigTracer() override;

    void   appendPoint( cvf::Vec3d position, cvf::Vec3d direction );
    size_t size() const;
    double totalDistance() const;

    void reverse();

    const std::vector<RigTracerPoint>& tracerPoints() const;

private:
    std::vector<RigTracerPoint> m_points;
};
