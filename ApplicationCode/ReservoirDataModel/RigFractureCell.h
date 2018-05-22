/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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
///  
//==================================================================================================
class RigFractureCell 
{

public:
    RigFractureCell(std::vector<cvf::Vec3d> polygon, size_t i, size_t j);

    const std::vector<cvf::Vec3d>& getPolygon() const;
    double                  getConductivtyValue() const;
    size_t                  getI() const;
    size_t                  getJ() const;

    bool                    hasNonZeroConductivity() const;
    void                    setConductivityValue(double cond);

    double                  cellSizeX() const;
    double                  cellSizeZ() const;

private:
    std::vector<cvf::Vec3d> m_polygon;
    double                  m_concutivityValue;
    size_t                  m_i;
    size_t                  m_j;
};
