/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018- Statoil ASA
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

class RiaLineArcWellPathCalculator
{
public:
    struct WellTarget
    {
        cvf::Vec3d targetPointXYZ;
        bool isTangentConstrained;
        double azimuth;
        double inclination;

        double radius1;
        double radius2;
    };

    RiaLineArcWellPathCalculator(const cvf::Vec3d& referencePointXyz, 
                                 const std::vector<RiaLineArcWellPathCalculator::WellTarget>& targets);

    struct WellTargetStatus
    {
        bool hasDerivedTangent;
        double resultAzimuth;
        double resultInclination;

        bool hasOverriddenRadius1;
        double resultRadius1;
        bool hasOverriddenRadius2;
        double resultRadius2;
    };

    cvf::Vec3d                           startTangent() const { return m_startTangent; }
    const std::vector<cvf::Vec3d>&       lineArcEndpoints() const { return m_lineArcEndpoints;} 
    const std::vector<WellTargetStatus>& targetStatuses() const { return m_targetStatuses;}
 
private:
    cvf::Vec3d                    m_startTangent;
    std::vector<cvf::Vec3d>       m_lineArcEndpoints;
    std::vector<WellTargetStatus> m_targetStatuses;
};

