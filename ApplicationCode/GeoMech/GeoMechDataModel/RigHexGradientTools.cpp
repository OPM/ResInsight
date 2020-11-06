/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RigHexGradientTools.h"

#include "../cafHexInterpolator/cafHexInterpolator.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<cvf::Vec3d, 8> RigHexGradientTools::gradients( const std::array<cvf::Vec3d, 8>& hexCorners,
                                                          const std::array<double, 8>&     values )
{
    //
    std::array<cvf::Vec3d, 8> gradientsUVW;
    gradientsUVW[0] = cvf::Vec3d( forwardFD( values, 0, 1 ), forwardFD( values, 0, 3 ), forwardFD( values, 0, 4 ) );
    gradientsUVW[1] = cvf::Vec3d( forwardFD( values, 0, 1 ), forwardFD( values, 1, 2 ), forwardFD( values, 1, 5 ) );
    gradientsUVW[2] = cvf::Vec3d( forwardFD( values, 3, 2 ), forwardFD( values, 1, 2 ), forwardFD( values, 2, 6 ) );
    gradientsUVW[3] = cvf::Vec3d( forwardFD( values, 2, 3 ), forwardFD( values, 0, 3 ), forwardFD( values, 3, 7 ) );
    gradientsUVW[4] = cvf::Vec3d( forwardFD( values, 4, 5 ), forwardFD( values, 4, 7 ), forwardFD( values, 0, 4 ) );
    gradientsUVW[5] = cvf::Vec3d( forwardFD( values, 4, 5 ), forwardFD( values, 5, 6 ), forwardFD( values, 1, 5 ) );
    gradientsUVW[6] = cvf::Vec3d( forwardFD( values, 7, 6 ), forwardFD( values, 5, 6 ), forwardFD( values, 2, 6 ) );
    gradientsUVW[7] = cvf::Vec3d( forwardFD( values, 7, 6 ), forwardFD( values, 4, 7 ), forwardFD( values, 3, 7 ) );

    std::array<cvf::Vec3d, 8> NC;
    NC[0] = { -1, -1, -1 };
    NC[1] = { 1, -1, -1 };
    NC[2] = { 1, 1, -1 };
    NC[3] = { -1, 1, -1 };
    NC[4] = { -1, -1, 1 };
    NC[5] = { 1, -1, 1 };
    NC[6] = { 1, 1, 1 };
    NC[7] = { -1, 1, 1 };

    std::array<cvf::Vec3d, 8> gradientsXYZ;

    for ( int i = 0; i < 8; i++ )
    {
        bool       isInvertPossible = false;
        cvf::Mat3d jacobian         = caf::HexInterpolator::jacobi( hexCorners, NC[i] );
        jacobian.transpose();
        gradientsXYZ[i] = gradientsUVW[i].getTransformedVector( jacobian.getInverted( &isInvertPossible ) );
    }

    return gradientsXYZ;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigHexGradientTools::forwardFD( const std::array<double, 8>& values, int from, int to )
{
    double h = 2.0;
    return ( values[to] - values[from] ) / h;
}
