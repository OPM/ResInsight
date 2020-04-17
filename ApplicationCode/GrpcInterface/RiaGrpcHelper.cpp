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
//////////////////////////////////////////////////////////////////////////////////
#include "RiaGrpcHelper.h"

//--------------------------------------------------------------------------------------------------
/// Convert internal ResInsight representation of cells with negative depth to positive depth.
//--------------------------------------------------------------------------------------------------
void RiaGrpcHelper::convertVec3dToPositiveDepth( cvf::Vec3d* vec )
{
    double& z = vec->z();
    z *= -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcHelper::setCornerValues( rips::Vec3d* out, const cvf::Vec3d& in )
{
    out->set_x( in.x() );
    out->set_y( in.y() );
    out->set_z( in.z() );
}
