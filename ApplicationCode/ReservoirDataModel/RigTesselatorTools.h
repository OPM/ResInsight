/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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


//==================================================================================================
/// 
//==================================================================================================
class RigEllipsisTesselator
{
public:
    RigEllipsisTesselator(size_t numSlices);

    void tesselateEllipsis(float a, float b, std::vector<cvf::uint>* triangleIndices, std::vector<cvf::Vec3f>* nodeCoords);

private:
    void computeCirclePoints(size_t numSlices);

private:
    std::vector<cvf::Vec3f> m_circlePoints;
    std::vector<cvf::uint>  m_circleConnectivities;
};
