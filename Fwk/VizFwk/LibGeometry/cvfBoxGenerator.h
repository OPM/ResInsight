//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#pragma once

#include "cvfVector3.h"

namespace cvf {

class GeometryBuilder;


//==================================================================================================
//
// Generates an axis aligned box
//
//==================================================================================================
class BoxGenerator
{
public:
    BoxGenerator();

    void    setMinMax(const Vec3d& minCoord, const Vec3d& maxCoord);
    void    setOriginAndExtent(const Vec3d& origin, const Vec3d& extent);
    void    setCenterAndExtent(const Vec3d& center, const Vec3d& extent);

    void    setSubdivisions(uint subdivX, uint subdivY, uint subdivZ);

    void    generate(GeometryBuilder* builder);

private:
    Vec3d   m_minCoord; 
    Vec3d   m_maxCoord;
    Vec3ui  m_subdivisions; // Number of cells/quads in each direction. Default (1, 1, 1)
};


}
