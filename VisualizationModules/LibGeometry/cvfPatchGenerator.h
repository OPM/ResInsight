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
// Generates 2D patches
//
//==================================================================================================
class PatchGenerator
{
public:
    PatchGenerator();

    void    setOrigin(const Vec3d& origin);
    void    setAxes(const Vec3d& axisU, const Vec3d& axisV);
    void    setExtent(double extentU, double extentV);
    void    setSubdivisions(uint subdivU, uint subdivV);

    void    setQuads(bool useQuads);
    void    setWindingCCW(bool windingCCW);

    void    generate(GeometryBuilder* builder);

private:
    Vec3d   m_origin;       // Origin. Default (0, 0, 0)
    Vec3d   m_axisU;        // First axis of patch. Default is global X-axis
    Vec3d   m_axisV;        // Second axis of patch. Default is global Y-axis
    double  m_extentU;      // Extent along U axis
    double  m_extentV;      // Extent along V axis 
    uint    m_cellCountU;   // Number of cells/quads in each direction. Default 1
    uint    m_cellCountV;   // :

    bool    m_useQuads;     // If true, quads will be generated, otherwise triangles. Default is quads
    bool    m_windingCCW;   // Winding of the generated quads. Controls which side of the patch will be front facing. 
};


}
