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
#pragma once

#include <array>
#include "cvfVector3.h"
#include <cmath>

// Y - North,  X - East, Z - up but depth is negative Z
// azi is measured from the Northing (Y) Axis in Clockwise direction looking down
// inc is measured from the negative Z (depth) axis
 
class RiaOffshoreSphericalCoords
{
public:
    explicit RiaOffshoreSphericalCoords(const cvf::Vec3f& vec)
    {
        // Azimuth: 
        if (vec[0] == 0.0f &&  vec[1] == 0.0 ) incAziR[1] = 0.0f;
        else incAziR[1] = atan2(vec[0], vec[1]); // atan2(Y, X)      

        // R
        incAziR[2] = vec.length();

        // Inclination from vertical down
        if (incAziR[2] == 0) incAziR[0] = 0.0f;
        else incAziR[0] = acos(-vec[2]/incAziR[2]);

    }

    explicit RiaOffshoreSphericalCoords(const cvf::Vec3d& vec)
    {
        // Azimuth: 
        if (vec[0] == 0.0 &&  vec[1] == 0.0 ) incAziR[1] = 0.0;
        else incAziR[1] = atan2(vec[0], vec[1]); // atan2(Y, X)      

                                                 // R
        incAziR[2] = vec.length();

        // Inclination from vertical down
        if (incAziR[2] == 0) incAziR[0] = 0.0;
        else incAziR[0] = acos(-vec[2]/incAziR[2]);

    }
    double inc() const { return incAziR[0];}
    double azi() const { return incAziR[1];}
    double r()   const { return incAziR[2];}

    // Note that this is a double function, while the rest of the class is float.
    // Todo: Convert class to a template to enable float and double versions of everything
    static cvf::Vec3d unitVectorFromAziInc(double azimuth, double inclination)
    {
        return cvf::Vec3d(sin(azimuth)*sin(inclination),
                          cos(azimuth)*sin(inclination),
                          -cos(inclination));
    }

private:
    std::array<double, 3> incAziR;
};

