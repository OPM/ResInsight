/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "cafPdmPointer.h"

#include "cvfBase.h"
#include "cvfVector3.h"
#include "cvfMatrix4.h"

#include <vector>

class RimFishbonesMultipleSubs;

//==================================================================================================
/// 
/// 
//==================================================================================================
class RigFisbonesGeometry
{
public:
    explicit RigFisbonesGeometry(RimFishbonesMultipleSubs* fishbonesSub);

    std::vector<std::pair<cvf::Vec3d, double>> coordsForLateral(size_t subIndex, size_t lateralIndex) const;
    
private:
    void computeLateralPositionAndOrientation(size_t subIndex, size_t lateralIndex,
                                              cvf::Vec3d* startCoord, cvf::Vec3d* startDirection,
                                              cvf::Mat4d* buildAngleMatrix) const;
    
    static std::vector<std::pair<cvf::Vec3d, double>> computeCoordsAlongLateral(double startMeasuredDepth, double lateralLength,
                                             const cvf::Vec3d& startCoord, const cvf::Vec3d& startDirection,
                                             const cvf::Mat4d& buildAngleMatrix);

    static cvf::Vec3d closestMainAxis(const cvf::Vec3d& vec);

private:
    caf::PdmPointer<RimFishbonesMultipleSubs> m_fishbonesSub;
};

