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
///  
//==================================================================================================
class RigSimulationWellCoordsAndMD
{
public:
    explicit RigSimulationWellCoordsAndMD(const std::vector<cvf::Vec3d>& wellPathPoints);

    const std::vector<cvf::Vec3d>&  wellPathPoints() const;
    const std::vector<double>&      measuredDepths() const;

    cvf::Vec3d                      interpolatedPointAlongWellPath(double measuredDepth) const;
    double                          locationAlongWellCoords(const cvf::Vec3d& position) const;

private:
    void                            computeMeasuredDepths();

private:
    std::vector<cvf::Vec3d> m_wellPathPoints;
    std::vector<double>     m_measuredDepths;
};
