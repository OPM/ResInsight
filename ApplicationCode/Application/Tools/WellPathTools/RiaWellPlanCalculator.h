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

class RiaWellPlanCalculator
{
public:
    RiaWellPlanCalculator(const cvf::Vec3d& startTangent, 
                          const std::vector<cvf::Vec3d>& lineArcEndPoints);

    struct WellPlanSegment
    {
        double MD; 
        double CL; 
        double inc;
        double azi;
        double TVD; 
        double NS; 
        double EW; 
        double dogleg; 
        double build; 
        double turn;
    };

    const std::vector<WellPlanSegment>& wellPlan() const { return m_wpResult; } 

private:
    void addSegment(cvf::Vec3d t1, cvf::Vec3d p1, cvf::Vec3d p2, cvf::Vec3d* endTangent);
    void addLineSegment(cvf::Vec3d p1, cvf::Vec3d p2, cvf::Vec3d* endTangent);
    void addArcSegment(cvf::Vec3d t1, cvf::Vec3d p1, cvf::Vec3d p2, cvf::Vec3d* endTangent);

    cvf::Vec3d               m_startTangent;
    std::vector<cvf::Vec3d>  m_lineArcEndPoints;

    std::vector<WellPlanSegment> m_wpResult;

};

