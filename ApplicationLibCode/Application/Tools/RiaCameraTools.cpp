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
///
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
///
//--------------------------------------------------------------------------------------------------
RiaCameraTools::Orientation RiaCameraTools::fromViewDirection( RiaDefines::ViewDirection viewDirection )
{
    const cvf::Vec3d zUp( 0, 0, 1 );
    const cvf::Vec3d yUp( 0, 1, 0 );

    switch ( viewDirection )
    {
        case RiaDefines::ViewDirection::NORTH:
            return { cvf::Vec3d( 0, -1, 0 ), zUp };
        case RiaDefines::ViewDirection::SOUTH:
            return { cvf::Vec3d( 0, 1, 0 ), zUp };
        case RiaDefines::ViewDirection::EAST:
            return { cvf::Vec3d( -1, 0, 0 ), zUp };
        case RiaDefines::ViewDirection::WEST:
            return { cvf::Vec3d( 1, 0, 0 ), zUp };
        case RiaDefines::ViewDirection::TOP:
            return { cvf::Vec3d( 0, 0, -1 ), yUp };
        case RiaDefines::ViewDirection::BOTTOM:
            return { cvf::Vec3d( 0, 0, 1 ), yUp };
    }

    return { cvf::Vec3d( 0, 0, -1 ), yUp };
}
