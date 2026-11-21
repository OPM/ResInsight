/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RiaViewDefines.h"

#include "cvfVector3.h"

//==================================================================================================
/// Camera orientations in display coordinates: x = east, y = north, z = up.
//==================================================================================================
class RiaCameraTools
{
public:
    struct Orientation
    {
        cvf::Vec3d direction; // Unit vector pointing from the camera towards the model
        cvf::Vec3d up; // Unit vector pointing up on the screen, orthogonal to direction
    };

    // Azimuth is the compass bearing of the horizontal part of the viewing direction (0 = looking north,
    // 90 = looking east). Tilt is the angle between the viewing direction and straight down (0 = looking
    // straight down, 90 = looking horizontally).
    static Orientation fromAzimuthAndTilt( double azimuthDegrees, double tiltDegrees );

    // Same orientations as the "View From ..." commands in the GUI. NORTH is viewed from the north,
    // looking south.
    static Orientation fromViewFrom( RiaDefines::ViewFrom viewFrom );
};
