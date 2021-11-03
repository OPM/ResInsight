/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-   Equinor ASA
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

#include <cvfArray.h>
#include <cvfColor3.h>
#include <cvfVector3.h>
#include <vector>

namespace cvf
{
class Part;
} // namespace cvf

//==================================================================================================
///
//==================================================================================================
class RivBoxGeometryGenerator
{
public:
    static cvf::ref<cvf::Part> createBoxFromVertices( const std::vector<cvf::Vec3f>& vertices, const cvf::Color3f color );

private:
    RivBoxGeometryGenerator(){};
};
