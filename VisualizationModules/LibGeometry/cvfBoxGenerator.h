//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
