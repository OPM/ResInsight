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

#pragma once

#include "RigHexGradientTools.h"

#include "cvfVector3.h"

#include <array>

//==================================================================================================
///
//==================================================================================================

class RigHexGradientTools
{
public:
    static std::array<cvf::Vec3d, 8> gradients( const std::array<cvf::Vec3d, 8>& hexCorners,
                                                const std::array<double, 8>&     values );

private:
    // Private to avoid instantiation
    RigHexGradientTools();

    static double forwardFD( const std::array<double, 8>& values, int from, int to );
};
