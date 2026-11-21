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

#include "RiaCameraTools.h"

#include "cvfMath.h"

//--------------------------------------------------------------------------------------------------
/// Returns the camera orientation for a compass bearing and a tilt from straight down.
/// Azimuth is the bearing of the horizontal part of the viewing direction (0 = looking north,
/// 90 = looking east). Tilt is the angle between the viewing direction and straight down
/// (0 = looking straight down, 90 = looking horizontally). The up vector points upwards on the screen.
//--------------------------------------------------------------------------------------------------
RiaCameraTools::Orientation RiaCameraTools::fromAzimuthAndTilt( double azimuthDegrees, double tiltDegrees )
{
    const double azimuth = cvf::Math::toRadians( azimuthDegrees );
    const double tilt    = cvf::Math::toRadians( tiltDegrees );

    const double sinAzimuth = cvf::Math::sin( azimuth );
    const double cosAzimuth = cvf::Math::cos( azimuth );
    const double sinTilt    = cvf::Math::sin( tilt );
    const double cosTilt    = cvf::Math::cos( tilt );

    Orientation orientation;
    orientation.direction = cvf::Vec3d( sinAzimuth * sinTilt, cosAzimuth * sinTilt, -cosTilt );
    orientation.up        = cvf::Vec3d( sinAzimuth * cosTilt, cosAzimuth * cosTilt, sinTilt );
    return orientation;
}

//--------------------------------------------------------------------------------------------------
/// Returns the camera orientation used by the "View From ..." commands in the GUI. The enum names the
/// side the model is viewed from, so NORTH looks towards the south. Horizontal views have z up, while top
/// and bottom views have y (north) up.
//--------------------------------------------------------------------------------------------------
RiaCameraTools::Orientation RiaCameraTools::fromViewFrom( RiaDefines::ViewFrom viewFrom )
{
    switch ( viewFrom )
    {
        case RiaDefines::ViewFrom::NORTH:
            return { -cvf::Vec3d::Y_AXIS, cvf::Vec3d::Z_AXIS };
        case RiaDefines::ViewFrom::SOUTH:
            return { cvf::Vec3d::Y_AXIS, cvf::Vec3d::Z_AXIS };
        case RiaDefines::ViewFrom::EAST:
            return { -cvf::Vec3d::X_AXIS, cvf::Vec3d::Z_AXIS };
        case RiaDefines::ViewFrom::WEST:
            return { cvf::Vec3d::X_AXIS, cvf::Vec3d::Z_AXIS };
        case RiaDefines::ViewFrom::TOP:
            return { -cvf::Vec3d::Z_AXIS, cvf::Vec3d::Y_AXIS };
        case RiaDefines::ViewFrom::BOTTOM:
            return { cvf::Vec3d::Z_AXIS, cvf::Vec3d::Y_AXIS };
    }

    return { -cvf::Vec3d::Z_AXIS, cvf::Vec3d::Y_AXIS };
}
