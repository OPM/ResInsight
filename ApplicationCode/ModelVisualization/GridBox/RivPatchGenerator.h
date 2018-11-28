/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "cvfBase.h"
#include "cvfVector3.h"

#include <vector>

namespace cvf
{
    class GeometryBuilder;
}

//==================================================================================================
//
// Generates 2D patches based on predefined coordinates along u and v axis
// Inspired by cvf::PatchGenerator
//
//==================================================================================================
class RivPatchGenerator
{
public:
    RivPatchGenerator();

    void    setOrigin(const cvf::Vec3d& origin);
    void    setAxes(const cvf::Vec3d& axisU, const cvf::Vec3d& axisV);
    void    setSubdivisions(const std::vector<double>& uValues, const std::vector<double>& vValues);

    void    generate(cvf::GeometryBuilder* builder);

private:
    cvf::Vec3d   m_origin;       // Origin. Default (0, 0, 0)
    cvf::Vec3d   m_axisU;        // First axis of patch. Default is global X-axis
    cvf::Vec3d   m_axisV;        // Second axis of patch. Default is global Y-axis

    std::vector<double> m_uValues;
    std::vector<double> m_vValues;

    bool    m_useQuads;     // If true, quads will be generated, otherwise triangles. Default is quads
    bool    m_windingCCW;   // Winding of the generated quads. Controls which side of the patch will be front facing. 
};

